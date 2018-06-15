#ifndef POLLER_HH
#define POLLER_HH

#include <unordered_map>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <boost/thread/mutex.hpp>
#include "reactor.hh"

class Poller
{
	struct FDEntry
	{
		boost::intrusive_ptr<Reactor> reactor;
		bool registered;
		
		FDEntry()
			: registered(false) {}
	};
	
	int numThreads;
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::vector<FDEntry> fdEntries;
	boost::mutex entriesMutex;
	
	volatile bool alive;
	
public:
	static const uint32_t IN_EVENTS  = EPOLLIN | EPOLLRDHUP;
	static const uint32_t OUT_EVENTS = EPOLLOUT;
	
	Poller(int numThreads, int cpuOffset, size_t expectedFDs = 1024);
	
	void ensureFit(int fd);
	
	void add(boost::intrusive_ptr<Reactor> reactor, int fd, uint32_t events);

	void rearm(boost::intrusive_ptr<Reactor> reactor, int fd, uint32_t events);

	void remove(int fd);
	
	void pleaseStop();
	
	void join();
	
	static void threadFun(Poller *poller);
	
	~Poller() {}
};

#endif // POLLER_HH
