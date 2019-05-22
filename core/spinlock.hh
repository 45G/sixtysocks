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
		return !held.exchange(true, std::memory_order_acquire);
	}

	void unlock()
	{
		held.store(false, std::memory_order_release);
	}
};

#endif // SPINLOCK_HH
