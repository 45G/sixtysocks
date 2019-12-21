#include <system_error>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "poller.hh"
#include "reactor.hh"

using namespace std;
using boost::intrusive_ptr;

Poller::Poller(int numThreads, int cpuOffset, size_t expectedFDs)
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

void Poller::assign(intrusive_ptr<Reactor> reactor)
{
	try
	{
		reactor->start();
	}
	catch (RescheduleException &resched)
	{
		add(reactor, resched.getFD(), resched.getEvents());
	}
	catch (exception &ex)
	{
		cerr << "Caught exception; killing reactor: " << ex.what() << endl;
		reactor->deactivate();
	}
}

void Poller::add(intrusive_ptr<Reactor> reactor, int fd, uint32_t events)
{
	tbb::spin_mutex::scoped_lock scopedLock(reactor->deactivationLock);

	if (fd < 0 || !reactor->isActive())
		return;

	if (fd >= (int)fdEntries.size())
		throw runtime_error("Maximum number of FDs exceeded");
	
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
	
	fdEntries[fd] = { reactor, true };
}

void Poller::remove(int fd, bool force)
{
	if ((fd < 0 || !fdEntries[fd].registered) && !force)
		return;

	int rc = epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, nullptr);
	if (rc < 0 && errno != ENOENT)
		throw system_error(errno, system_category());

	fdEntries[fd] = { nullptr, false };
}

void Poller::stop()
{
	alive = false;
	
	for (FDEntry &entry: fdEntries)
	{
		if (entry.reactor)
			entry.reactor->deactivate();
	}
}

void Poller::join()
{
	for (thread &t: threads)
		t.join();
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
		{
			if (errno == EINTR)
				continue;
			throw system_error(errno, system_category());
		}
		
		intrusive_ptr<Reactor> reactor = poller->fdEntries[event.data.fd].reactor;
		poller->fdEntries[event.data.fd].reactor = nullptr;
		
		if (!reactor->isActive())
			return;
		
		poller->runAs(reactor.get(), [&]() {
			reactor->process(event.data.fd, event.events);
		});
	}
}
