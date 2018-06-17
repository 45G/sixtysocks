#ifndef REACTOR_HH
#define REACTOR_HH

#include <stdint.h>
#include <atomic>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/intrusive_ptr.hpp>

class Poller;

class Reactor: public boost::intrusive_ref_counter<Reactor>
{
protected:
	Poller *poller;
	volatile bool active;
	
public:
	Reactor(Poller *poller)
		: poller(poller), active(true) {}
	
	virtual void process(int fd, uint32_t events) = 0;
	
	virtual void deactivate();
	
	bool isActive() const
	{
		return active;
	}

	Poller *getPoller() const
	{
		return poller;
	}
	
	virtual ~Reactor();
};

#endif // REACTOR_HH
