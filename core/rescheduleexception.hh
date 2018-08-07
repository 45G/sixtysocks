#ifndef RESCHEDULEEXCEPTION_HH
#define RESCHEDULEEXCEPTION_HH

#include <stdint.h>
#include <exception>

class Reactor;

class RescheduleException: public std::exception
{
	int fd;
	uint32_t events;
	
public:
	RescheduleException(int fd, uint32_t events)
		: fd(fd), events(events) {}
	
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
