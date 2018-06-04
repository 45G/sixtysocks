#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <system_error>
#include "proxifieracceptreactor.hh"
#include "poller.hh"
#include "proxiferupstreamreactor.hh"

#include <iostream>

using namespace std;

void ProxifierAcceptReactor::process(Poller *poller, uint32_t events)
{
	if (!alive)
		return;
	
	if (events & EPOLLERR)
		processError();
	
	if (events & EPOLLIN)
	{
		while (true)
		{
			int newFD = accept(listenFD, NULL, NULL);
			if (newFD < 0)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
					break;
				else
					processError(errno);
				continue;
			}
			int rc = fcntl(newFD, F_SETFD, O_NONBLOCK);
			if (rc < 0)
				goto error;
			
			// Tolerable error
			const static int one = 1;
			setsockopt(newFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int));
			
			poller->add(new ProxiferUpstreamReactor(newFD), newFD, EPOLLIN);
			
			continue;
			
error:
			close(newFD);
		}
		
		poller->add(this, listenFD, EPOLLIN);
	}
}

ProxifierAcceptReactor::~ProxifierAcceptReactor()
{
	close(listenFD);
}
