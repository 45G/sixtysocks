#ifndef SYNCEDTOKENSTUFF_H
#define SYNCEDTOKENSTUFF_H

#include <socks6util/idempotence.hh>
#include "../core/spinlock.hh"

class SyncedTokenBank: S6U::TokenBank
{
	Spinlock spinlock;

public:
	SyncedTokenBank(uint32_t base, uint32_t size, uint32_t lowWatermark, uint32_t highWatermark)
		: TokenBank(base, size, lowWatermark, highWatermark) {}
	
	SOCKS6TokenExpenditureCode withdraw(uint32_t token)
	{
		std::lock_guard<Spinlock> lock(spinlock);
		return TokenBank::withdraw(token);
	}
	
	void renew()
	{
		std::lock_guard<Spinlock> lock(spinlock);
		TokenBank::renew();
	}
	
	void getWindow(uint32_t *base, uint32_t *size)
	{
		std::lock_guard<Spinlock> lock(spinlock);
		*base = getBase();
		*size = getSize();
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
		std::lock_guard<Spinlock> lock(spinlock);
		return S6U::TokenWallet::extract(token);
	}
	
	void updateWindow(uint32_t newBase, uint32_t newSize)
	{
		std::lock_guard<Spinlock> lock(spinlock);
		return S6U::TokenWallet::updateWindow(newBase, newSize);
	}
	
	void updateWindow(const S6M::OptionSet *optionSet)
	{
		std::lock_guard<Spinlock> lock(spinlock);
		return S6U::TokenWallet::updateWindow(optionSet);
	}
	
	uint32_t remaining() const
	{
		std::lock_guard<Spinlock> lock(spinlock);
		return S6U::TokenWallet::remaining();
	}
};

#endif // SYNCEDTOKENSTUFF_H
