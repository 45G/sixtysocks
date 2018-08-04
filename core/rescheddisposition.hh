#ifndef RESCHEDDISPOSITION_HH
#define RESCHEDDISPOSITION_HH

#include <stdint.h>
#include <exception>

class ReschedDisposition: public std::exception
{
	int fd;
	uint32_t events;
	
public:
	ReschedDisposition(int fd, uint32_t events)
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


#endif // RESCHEDDISPOSITION_HH
