#ifndef POLLER_HH
#define POLLER_HH

#include <unordered_map>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <boost/thread/mutex.hpp>

class Reactor;

class Poller
{
	int numThreads;
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::vector<Reactor *> reactors;
	boost::mutex reactorsMutex;
	
	volatile bool alive;
	
public:
	static const uint32_t IN_EVENTS  = EPOLLIN | EPOLLRDHUP;
	static const uint32_t OUT_EVENTS = EPOLLOUT;
	
	Poller(int numThreads, int cpuOffset);
	
	void add(Reactor *reactor, int fd, uint32_t events);

	void rearm(int fd, uint32_t events);

	void remove(int fd);
	
	void stop();
	
	void join();
	
	static void threadFun(Poller *poller);
	
	~Poller() {}
};

#endif // POLLER_HH
