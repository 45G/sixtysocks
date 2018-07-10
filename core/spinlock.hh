#ifndef SPINLOCK_HH
#define SPINLOCK_HH

#include <boost/atomic.hpp>

class Spinlock
{
	boost::atomic<bool> held;

public:
	Spinlock()
		:held(false) {}

	void acquire()
	{
		while (held.exchange(true, boost::memory_order_acquire));
	}

	bool attempt()
	{
		return held.exchange(true, boost::memory_order_acquire) == false;
	}

	void release()
	{
		held = 0;
	}
};

class ScopedSpinlock
{
	Spinlock *spinlock;
	
public:
	ScopedSpinlock(Spinlock *spinlock)
		: spinlock(spinlock)
	{
		spinlock->acquire();
	}
	
	~ScopedSpinlock()
	{
		spinlock->release();
	}
};


#endif // SPINLOCK_HH
