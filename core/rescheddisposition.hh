#ifndef RESCHEDDISPOSITION_HH
#define RESCHEDDISPOSITION_HH

#include <stdint.h>

struct ReschedDisposition
{
	int fd;
	uint32_t events;

	ReschedDisposition(int fd, uint32_t events)
		: fd(fd), events(events) {}
};


#endif // RESCHEDDISPOSITION_HH
