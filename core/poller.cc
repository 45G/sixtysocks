#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
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
		throw system_error(errno, system_category());
}

void Poller::add(Reactor *reactor, int fd, uint32_t events)
{
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.ptr = reactor;
	
	reactor->use();
	
	int rc = epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
	if (rc < 0)
		throw system_error(errno, system_category());
	
	fds.reserve(fd);
	fds[fd] = reactor;
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
			throw system_error(rc, system_category());
	}
		
}

void Poller::stop()
{
	alive = false;
	
	for (int i = 0; i < (int)fds.size(); i++)
	{
		if (!fds[i])
			continue;
		
		int rc = epoll_ctl(epollFD, EPOLL_CTL_DEL, i, NULL);
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		
		fds[i]->deactivate();
		fds[i]->unuse();
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
		
		Reactor *reactor = reinterpret_cast<Reactor *>(event.data.ptr);
		poller->fds[reactor->getFD()] = NULL;
		
		try
		{
			reactor->process(poller, event.events);
		}
		catch (...)
		{
			reactor->deactivate();
		}
		
		reactor->unuse();
	}
}
