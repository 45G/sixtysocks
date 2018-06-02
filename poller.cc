#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <boost/foreach.hpp>
#include "poller.hh"
#include "reactor.hh"

using namespace std;



Poller::Poller(int numThreads, int cpuOffset)
	: numThreads(numThreads), cpuOffset(cpuOffset), alive(true)
{
	threads.reserve(numThreads);
	epollFD = epoll_create(1337);
	if (epollFD < 0)
		throw system_error(errno, std::system_category());
}

void Poller::add(Reactor *fsm)
{
	epoll_event event;
	event.events = fsm->desiredEvents() | EPOLLONESHOT;
	event.data.ptr = fsm;
	
	fsm->use();
	
	int rc = epoll_ctl(epollFD, EPOLL_CTL_ADD, fsm->getFD(), &event);
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	fds.reserve(fsm->getFD());
	fds[fsm->getFD()] = true;
}

void Poller::start()
{
	for (int i = 0; i < numThreads; i++)
	{
		threads[i] = thread(threadFun, this);
		
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i + cpuOffset, &cpuset);
		int rc = pthread_setaffinity_np(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
		if (rc > 0)
			throw system_error(rc, std::system_category());
	}
		
}

void Poller::stop()
{
	alive = false;
	
	for (int i = 0; i < (int)fds.size(); i++)
	{
		if (!fds[i])
			continue;
		
		epoll_event event;
		
		int rc = epoll_ctl(epollFD, EPOLL_CTL_DEL, i, &event);
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		
		Reactor *fsm = reinterpret_cast<Reactor *>(event.data.ptr);
		fsm->kill();
		fsm->unuse();
	}
	
	for (int i = 0; i < (int)threads.size(); i++)
		threads[i].join();
}

Poller::~Poller()
{
	
}

void Poller::threadFun(Poller *poller)
{
	while (poller->alive)
	{
		epoll_event event;
		
		int rc = epoll_wait(poller->epollFD, &event, 1, 2000);
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		if (rc == 0)
			continue;
		
		Reactor *fsm = reinterpret_cast<Reactor *>(event.data.ptr);
		poller->fds[fsm->getFD()] = false;
		
		fsm->process(poller, event.events);
		
		fsm->unuse();
	}
}
