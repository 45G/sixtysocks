#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <boost/foreach.hpp>
#include "poller.hh"
#include "reactor.hh"

using namespace std;

Poller::Poller(int numThreads, int cpuOffset)
	: numThreads(numThreads), alive(true)
{
	threads.reserve(numThreads);
	epollFD = epoll_create(1337);
	if (epollFD < 0)
		throw system_error(errno, system_category());
	reactors.reserve(1024);
	
//	for (int i = 0; i < numThreads; i++)
//	{
//		threads[i] = thread(threadFun, this);
		
//		if (cpuOffset < 0)
//			continue;
		
//		cpu_set_t cpuset;
//		CPU_ZERO(&cpuset);
//		CPU_SET(i + cpuOffset, &cpuset);
//		int rc = pthread_setaffinity_np(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
//		if (rc > 0)
//			throw system_error(rc, system_category());
//	}
}

void Poller::add(Reactor *reactor, int fd, uint32_t events)
{
	if (fd < 0)
		return;

	if (reactors.size() > fd && reactors[fd] != NULL)
		rearm(fd, events);
	
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.ptr = reactor;
	
	reactor->use();
	
	int rc = epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
	if (rc < 0)
	{
		reactor->unuse();
		throw system_error(errno, system_category());
	}
	
	size_t reqSize = reactors.size();
	while ((int)reqSize < fd + 1)
		reqSize *= 2;
	if (reactors.size() < reqSize)
	{
		//TODO: lock!
		reactors.reserve(reqSize);
	}
	reactors.reserve(fd + 1);

	reactors[fd] = reactor;
}

void Poller::rearm(int fd, uint32_t events)
{
	Reactor *reactor = reactors[fd];
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.ptr = reactor;

	reactor->use();

	int rc = epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &event);
	if (rc < 0)
	{
		reactor->unuse();
		throw system_error(errno, system_category());
	}
}

void Poller::remove(int fd)
{
	Reactor *reactor = reactors[fd];
	if (reactor == NULL)
		return;

	int rc = epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL);
	if (rc < 0)
	{
		throw system_error(errno, system_category());
	}
}

void Poller::stop()
{
	alive = false;
	
	for (int i = 0; i < (int)reactors.size(); i++)
	{
		if (!reactors[i])
			continue;
		
		reactors[i]->deactivate();
	}
}

void Poller::join()
{
	for (int i = 0; i < (int)threads.size(); i++)
		threads[i].join();
}

void Poller::threadFun(Poller *poller)
{
	while (poller->alive)
	{
		epoll_event event;
		
		int rc = epoll_wait(poller->epollFD, &event, 1, -1);
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		if (rc == 0)
			continue;
		
		Reactor *reactor = reinterpret_cast<Reactor *>(event.data.ptr);
		
		try
		{
			reactor->process();
		}
		catch (...)
		{
			reactor->deactivate();
		}
		
		reactor->unuse();
	}
}
