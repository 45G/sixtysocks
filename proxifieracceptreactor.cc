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

#include <iostream>

using namespace std;

ProxifierAcceptReactor::ProxifierAcceptReactor(int fd)
	: Reactor(fd)
{
	
}

void ProxifierAcceptReactor::process(Poller *poller, uint32_t events)
{
	if (!alive)
		return;
	
	if (events & EPOLLERR)
	{
		int err;
		socklen_t errLen;
		
		int rc = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
		if (rc < 0)
			throw system_error(errno, system_category());
		
		switch (err)
		{
		case ECONNABORTED:
		case EINTR:
		case EMFILE:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case EPROTO:
			break;
			
		default:
			throw system_error(err, system_category());
		}
	}
	
	while (true)
	{
		int newFD = accept(fd, NULL, NULL);
		if (newFD < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				poller->add(this, EPOLLIN);
			return;
		}
		int rc = fcntl(newFD, F_SETFD, O_NONBLOCK);
		if (rc < 0)
			throw system_error(errno, system_category());
		
		int one = 1;
		rc = setsockopt(newFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int));
		if (rc < 0)
			throw system_error(errno, system_category());
		
		//TODO
		cout << "new connection" << endl;
	}
}

ProxifierAcceptReactor::~ProxifierAcceptReactor()
{
	
}
