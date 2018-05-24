#ifndef POLLER_HH
#define POLLER_HH

#include <unordered_map>
#include <thread>
#include <vector>

class FSM;

class Poller
{
	int numThreads;
	int cpuOffset;
	
	int epollFD;
	
	std::vector<std::thread> threads;
	
	std::unordered_map<int, FSM *> fsms;
	
public:
	Poller(int numThreads, int cpuOffset);
	
	void add(FSM *fsm);
	
	void stop();
	
	~Poller();
};

#endif // POLLER_HH
