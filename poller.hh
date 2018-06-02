#ifndef POLLER_HH
#define POLLER_HH

#include <unordered_map>
#include <thread>
#include <vector>

class Reactor;

class Poller
{
	int numThreads;
	int cpuOffset;
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::vector<bool> fds;
	
	volatile bool alive;
	
public:
	Poller(int numThreads, int cpuOffset);
	
	void add(Reactor *fsm, uint32_t events);
	
	void start();
	
	void stop();
	
	static void threadFun(Poller *poller);
	
	~Poller();
};

#endif // POLLER_HH
