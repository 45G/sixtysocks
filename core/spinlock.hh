#ifndef SPINLOCK_HH
#define SPINLOCK_HH

#include <atomic>

class Spinlock
{
	std::atomic<bool> held { false };

public:
	void acquire()
	{
		while (held.exchange(true, std::memory_order_acquire));
	}

	bool attempt()
	{
		return held.exchange(true, std::memory_order_acquire) == false;
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
