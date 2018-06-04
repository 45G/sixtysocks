#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <system_error>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "proxiferupstreamer.hh"

#include <iostream>

using namespace std;

void Proxifier::process(Poller *poller, uint32_t events)
{
	(void)events;
	
	while (active)
	{
		int clientFD = accept(listenFD, NULL, NULL);
		if (clientFD < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
				break;
			else
				processError(errno);
			continue;
		}
		int rc = fcntl(clientFD, F_SETFD, O_NONBLOCK);
		if (rc < 0)
			deactivate();
		
		// Tolerable error
		const static int one = 1;
		setsockopt(clientFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int));
		
		ProxiferUpstreamer *upstreamReactor = NULL;
		try
		{
			upstreamReactor = new ProxiferUpstreamer(this, clientFD);
		}
		catch (bad_alloc)
		{
			// tolerable error
			close(clientFD);
			continue;
		}
		
		try
		{
			poller->add(upstreamReactor, clientFD, EPOLLIN | EPOLLRDHUP);
		}
		catch (exception)
		{
			delete upstreamReactor;
		}
	}
	
	poller->add(this, listenFD, EPOLLIN);
}

Proxifier::~Proxifier()
{
	close(listenFD);
}
