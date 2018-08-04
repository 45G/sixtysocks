#ifndef RESCHEDULEEXCEPTION_HH
#define RESCHEDULEEXCEPTION_HH

#include <stdint.h>
#include <exception>

class Reactor;

class RescheduleException: public std::exception
{
	Reactor *reactor;
	int fd;
	uint32_t events;
	
public:
	RescheduleException(Reactor *reactor, int fd, uint32_t events)
		: reactor(reactor), fd(fd), events(events) {}
	
	Reactor *getReactor() const
	{
		return reactor;
	}
	
	int getFD() const
	{
		return fd;
	}
	
	uint32_t getEvents() const
	{
		return events;
	}
};


#endif // RESCHEDULEEXCEPTION_HH
