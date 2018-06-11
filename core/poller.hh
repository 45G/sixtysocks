#ifndef POLLER_HH
#define POLLER_HH

#include <unordered_map>
#include <thread>
#include <vector>
#include <sys/epoll.h>

class Reactor;

class Poller
{
	int numThreads;
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::vector<Reactor *> fds;
	
	volatile bool alive;
	
public:
	static const uint32_t IN_EVENTS  = EPOLLIN | EPOLLRDHUP;
	static const uint32_t OUT_EVENTS = EPOLLOUT;
	
	Poller(int numThreads, int cpuOffset);
	
	void add(Reactor *reactor, int fd, uint32_t events);
	
	void stop();
	
	void join();
	
	static void threadFun(Poller *poller);
	
	~Poller() {}
};

#endif // POLLER_HH
