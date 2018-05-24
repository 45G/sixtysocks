#ifndef FSM_HH
#define FSM_HH

#include <stdint.h>

class FSM
{
protected:
	bool alive;
	int refCnt;
	int fd;
	
public:
	FSM(int fd)
		: alive(true), refCnt(1), fd(fd) {}
	
	virtual bool process(uint32_t events) = 0;
	
	void use();
	void unuse();
	
	void kill();
	
	int getFD() const
	{
		return fd;
	}
	
	virtual uint32_t desiredEvent() = 0;
};

#endif // FSM_HH
