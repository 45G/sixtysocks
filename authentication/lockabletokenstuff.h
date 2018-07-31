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

class SyncedTokenWallet: S6U::TokenWallet
{
	mutable Spinlock spinlock;
public:
	SyncedTokenWallet() {}
	
	SyncedTokenWallet(uint32_t base, uint32_t size)
		: TokenWallet(base, size) {}
	
	bool extract(uint32_t *token)
	{
		ScopedSpinlock lock(&spinlock); (void)lock;
		return S6U::TokenWallet::extract(token);
	}
	
	void updateWindow(uint32_t newBase, uint32_t newSize)
	{
		ScopedSpinlock lock(&spinlock); (void)lock;
		return S6U::TokenWallet::updateWindow(newBase, newSize);
	}
	
	void updateWindow(const S6M::OptionSet *optionSet)
	{
		ScopedSpinlock lock(&spinlock); (void)lock;
		return S6U::TokenWallet::updateWindow(optionSet);
	}
	
	uint32_t remaining() const
	{
		ScopedSpinlock lock(&spinlock); (void)lock;
		return S6U::TokenWallet::remaining();
	}
};

#endif // LOCKABLETOKENSTUFF_H
