#ifndef REACTOR_HH
#define REACTOR_HH

#include <stdint.h>
#include <atomic>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

class Poller;

class Reactor
{
protected:
	Poller *poller;
	bool active;
	std::atomic<int> refCnt;
	
public:
	Reactor(Poller *poller)
		: poller(poller), active(true), refCnt(0) {}
	
	virtual void process() = 0;
	
	void use()
	{
		refCnt++;
	}

	void unuse()
	{
		refCnt--;
		if (refCnt == 0)
			delete this;
	}
	
	void deactivate()
	{
		active = false;
	}

	Poller *getPoller() const
	{
		return poller;
	}
	
	virtual ~Reactor();
};

#endif // REACTOR_HH
