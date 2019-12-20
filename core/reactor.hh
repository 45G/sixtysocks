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
#include <tbb/spin_mutex.h>
#include <socks6util/socks6util.hh>
#include "rescheduleexception.hh"
#include "uniqfd.hh"
#include "streambuffer.hh"

class Poller;

class Reactor: public boost::intrusive_ref_counter<Reactor>
{
private:
	std::atomic<bool> active { true };
	tbb::spin_mutex deactivationLock;

protected:
	Poller *poller;

public:
	Reactor(Poller *poller)
		: poller(poller) {}
	
	virtual void start() = 0;

	virtual void process(int fd, uint32_t events) = 0;
	
	virtual void deactivate();
	
	bool isActive() const
	{
		return active;
	}
	
	template <typename T>
	bool runIfActive(T lambda)
	{
		tbb::spin_mutex::scoped_lock scopedLock(deactivationLock);
		if (!isActive())
			return false;
		lambda();
		return true;
	}

	Poller *getPoller() const
	{
		return poller;
	}
	
	virtual ~Reactor() = default;
};

#endif // REACTOR_HH
