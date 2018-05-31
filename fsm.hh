#ifndef FSM_HH
#define FSM_HH

#include <stdint.h>
#include <atomic>

class Poller;

class FSM
{
protected:
	bool alive;
	std::atomic<int> refCnt;
	int fd;
	
public:
	FSM(int fd)
		: alive(true), refCnt(0), fd(fd) {}
	
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
	
	int getFD() const
	{
		return fd;
	}
	
	virtual uint32_t desiredEvents() = 0;
	
	virtual ~FSM();
};

#endif // FSM_HH
