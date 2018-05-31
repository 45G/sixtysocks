#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "proxifieracceptfsm.hh"
#include "poller.hh"

#include <iostream>

using namespace std;

ProxifierAcceptFSM::ProxifierAcceptFSM(int fd)
	: FSM(fd)
{
	
}

void ProxifierAcceptFSM::process(Poller *poller, uint32_t events)
{
	if (!alive)
		return;
	
	if (events & EPOLLERR)
	{
		//TODO: error code
		kill();
		return;
	}
	
	while (true)
	{
		int newFD = accept(fd, NULL, NULL);
		if (newFD < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				poller->add(this);
			return;
		}
		int rc = fcntl(newFD, F_SETFD, O_NONBLOCK);
		if (rc < 0)
		{
			//TODO
		}
		
		int one = 1;
		rc = setsockopt(newFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int));
		if (rc < 0)
		{
			//TODO
		}
		//TODO
		cout << "new connection" << endl;
	}
}

uint32_t ProxifierAcceptFSM::desiredEvents()
{
	return EPOLLIN;
}

ProxifierAcceptFSM::~ProxifierAcceptFSM()
{
	
}
