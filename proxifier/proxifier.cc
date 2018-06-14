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
#include "proxifierupstreamer.hh"

#include <iostream>

using namespace std;

void Proxifier::process()
{
	while (active)
	{
		int clientFD = accept4(listenFD.fd, NULL, NULL, SOCK_NONBLOCK);
		if (clientFD < 0)
		{
			switch (errno)
			{
			case EWOULDBLOCK:
#if EWOULDBLOCK != EAGAIN
			case EAGAIN:
#endif
				goto resched;

			case EINTR:
			case ENETDOWN:
			case EPROTO:
			case ENOPROTOOPT:
			case EHOSTDOWN:
			case ENONET:
			case EHOSTUNREACH:
			case EOPNOTSUPP:
			case ENETUNREACH:
				break;
			default:
				processError(errno);
			}
			continue;
		}
		
		const static int one = 1;
		setsockopt(clientFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int)); // tolerable error
		
		ProxifierUpstreamer *upstreamReactor = NULL;
		try
		{
			upstreamReactor = new ProxifierUpstreamer(this, clientFD);
		}
		catch (bad_alloc)
		{
			close(clientFD); // tolerable error
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

resched:
	poller->add(this, listenFD.fd, EPOLLIN);
}

Proxifier::~Proxifier() {}
