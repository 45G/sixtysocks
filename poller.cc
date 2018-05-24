#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <boost/foreach.hpp>
#include "poller.hh"
#include "fsm.hh"

using namespace std;



Poller::Poller(int numThreads, int cpuOffset)
	: numThreads(numThreads), cpuOffset(cpuOffset)
{
	epollFD = epoll_create(1337);
	if (epollFD < 0)
		throw std::system_error(errno, std::system_category());
}

void Poller::add(FSM *fsm)
{
	fsms[fsm->getFD()] = fsm;
	epoll_event event;
	event.events = fsm
	
	epoll_ctl(epollFD, EPOLL_CTL_ADD, fsm->desiredEvent(), &event);
}

void Poller::stop()
{
	pair<int, FSM *> entry;
	BOOST_FOREACH(entry, fsms)
	{
		entry.second->kill();
		entry.second->unuse();
	}
	
	fsms.clear();
}

Poller::~Poller()
{
	
}
