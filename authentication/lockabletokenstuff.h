#ifndef LOCKABLETOKENSTUFF_H
#define LOCKABLETOKENSTUFF_H

#include <socks6util/socks6util_idempotence.hh>
#include "../core/spinlock.hh"

class LockableTokenBank: public S6U::TokenBank
{
	Spinlock spinlock;
public:
	LockableTokenBank(uint32_t base, uint32_t size, uint32_t lowWatermark, uint32_t highWatermark)
		: TokenBank(base, size, lowWatermark, highWatermark) {}
	
	void acquire()
	{
		spinlock.acquire();
	}
	
	void attempt()
	{
		spinlock.attempt();
	}
	
	void release()
	{
		spinlock.release();
	}
};

class LockableTokenWallet: public S6U::TokenWallet
{
	Spinlock spinlock;
public:
	LockableTokenWallet() {}
	
	LockableTokenWallet(uint32_t base, uint32_t size)
		: TokenWallet(base, size) {}
	
	void acquire()
	{
		spinlock.acquire();
	}
	
	void attempt()
	{
		spinlock.attempt();
	}
	
	void release()
	{
		spinlock.release();
	}
};

#endif // LOCKABLETOKENSTUFF_H
