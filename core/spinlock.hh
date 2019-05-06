#ifndef SPINLOCK_HH
#define SPINLOCK_HH

#include <atomic>
#include <mutex>

class Spinlock
{
	std::atomic<bool> held { false };

public:
	void lock()
	{
		while (held.exchange(true, std::memory_order_acquire));
	}

	bool try_lock()
	{
		if (held.exchange(true, std::memory_order_acquire))
		{
			std::atomic_thread_fence(std::memory_order_release);
			return false;
		}

		return true;
	}

	void unlock()
	{
		held.store(false, std::memory_order_release);
	}
};

typedef std::lock_guard<Spinlock> ScopedSpinlock;

//class ScopedSpinlock
//{
//	Spinlock *spinlock;
	
//public:
//	ScopedSpinlock(Spinlock *spinlock)
//		: spinlock(spinlock)
//	{
//		spinlock->lock();
//	}

//	~ScopedSpinlock()
//	{
//		spinlock->unlock();
//	}
//};


#endif // SPINLOCK_HH
