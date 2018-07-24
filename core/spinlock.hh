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
		held = false;
	}
};

class ScopedSpinlock
{
	Spinlock *spinlock;
	
public:
	ScopedSpinlock()
		: spinlock(NULL) {}
	
	ScopedSpinlock(Spinlock *spinlock)
		: spinlock(spinlock)
	{
		spinlock->acquire();
	}
	
	void reset()
	{
		*this = ScopedSpinlock();
	}
	
	~ScopedSpinlock()
	{
		if (spinlock != NULL)
			spinlock->release();
	}
};


#endif // SPINLOCK_HH
