#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <boost/foreach.hpp>
#include "poller.hh"
#include "reactor.hh"

using namespace std;
using namespace boost;

Poller::Poller(int numThreads, int cpuOffset, size_t expectedFDs)
	: numThreads(numThreads), alive(true)
{
	threads.reserve(numThreads);
	epollFD = epoll_create(1); // number doesn't matter
	if (epollFD < 0)
		throw system_error(errno, system_category());
	fdEntries.resize(expectedFDs);
	
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

void Poller::ensureFit(int fd)
{
	if ((int)fdEntries.size() < fd + 1)
	{
		size_t reqSize = fdEntries.size();
		do
		{
			reqSize *= 2;
		}
		while ((int)reqSize < fd + 1);

		entriesMutex.lock();
		if (fdEntries.size() < reqSize)
			fdEntries.resize(reqSize);
		entriesMutex.unlock();
	}
}

void Poller::add(intrusive_ptr<Reactor> reactor, int fd, uint32_t events)
{
	if (fd < 0 || !reactor->isActive())
		return;

	ensureFit(fd);
	
	int epollOp;
	if (fdEntries[fd].registered)
		epollOp = EPOLL_CTL_MOD;
	else
		epollOp = EPOLL_CTL_ADD;
	
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.fd = fd;
	
	int rc = epoll_ctl(epollFD, epollOp, fd, &event);
	if (rc < 0)
		throw system_error(errno, system_category());
	
	fdEntries[fd].reactor = reactor;
	fdEntries[fd].registered = true;
	
	/* work around race condition with Reactor::pleaseStop() */
	if (!reactor->isActive())
		remove(fd, true);
}

void Poller::remove(int fd, bool force)
{
	if ((!fdEntries[fd].registered || fd < 0) && !force)
		return;

	int rc = epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL);
	if (rc < 0 && errno != ENOENT)
		throw system_error(errno, system_category());

	fdEntries[fd].reactor = NULL;
	fdEntries[fd].registered = false;
}

void Poller::stop()
{
	alive = false;
	
	for (int i = 0; i < (int)fdEntries.size(); i++)
	{
		if (fdEntries[i].reactor == NULL)
			continue;
		
		fdEntries[i].reactor->deactivate();
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
		if (rc == 0)
			continue;
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		
		intrusive_ptr<Reactor> reactor = poller->fdEntries[event.data.fd].reactor;
		poller->fdEntries[event.data.fd].reactor = NULL;
		
		if (!reactor->isActive())
			return;
		
		try
		{
			reactor->process();
		}
		catch (...)
		{
			reactor->deactivate();
		}
	}
}
