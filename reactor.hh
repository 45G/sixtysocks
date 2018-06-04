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
	bool alive;
	std::atomic<int> refCnt;
	
public:
	Reactor()
		: alive(true), refCnt(0) {}
	
	virtual void process(Poller *poller, uint32_t events) = 0;
	
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
	
	void kill()
	{
		alive = false;
	}
	
	virtual int getFD() const = 0;
	
	virtual ~Reactor();
};

#endif // REACTOR_HH
