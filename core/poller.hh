#ifndef POLLER_HH
#define POLLER_HH

#include <atomic>
#include <unordered_map>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <exception>
#include <iostream>
#include "reactor.hh"

class Poller
{
	struct FDEntry
	{
		boost::intrusive_ptr<Reactor> reactor;
		bool registered = false;
	};
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::vector<FDEntry> fdEntries;
	
	std::atomic<bool> alive { true };
	
public:
	static constexpr uint32_t IN_EVENTS  = EPOLLIN | EPOLLRDHUP;
	static constexpr uint32_t OUT_EVENTS = EPOLLOUT;
	
	Poller(int numThreads, size_t expectedFDs = 1 << 17);
	
	template <typename T>
	void runAs(boost::intrusive_ptr<Reactor> reactor, T functor)
	{
		try
		{
			functor();
		}
		catch (RescheduleException &resched)
		{
			add(reactor, resched.getFD(), resched.getEvents());
		}
		catch (std::exception &ex)
		{
			std::cerr << "Caught exception; killing reactor: " << ex.what() << std::endl;
			reactor->deactivate();
		}
	}
	
	void assign(boost::intrusive_ptr<Reactor> reactor)
	{
		runAs(reactor, [&]() {
			reactor->start();
		});
	}
	
	void add(boost::intrusive_ptr<Reactor> reactor, int fd, uint32_t events);
	
	void remove(int fd, bool force = false);
	
	void stop();
	
	void join();
	
	static void threadFun(Poller *poller);
};

#endif // POLLER_HH
