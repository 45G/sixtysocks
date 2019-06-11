#ifndef SYNCEDTOKENSTUFF_H
#define SYNCEDTOKENSTUFF_H

#include <socks6util/idempotence.hh>
#include <tbb/spin_mutex.h>

class SyncedTokenBank: S6U::TokenBank
{
	tbb::spin_mutex spinlock;

public:
	SyncedTokenBank(uint32_t base, uint32_t size, uint32_t lowWatermark, uint32_t highWatermark)
		: TokenBank(base, size, lowWatermark, highWatermark) {}
	
	bool withdraw(uint32_t token)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return TokenBank::withdraw(token);
	}
	
	void renew()
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		TokenBank::renew();
	}
	
	std::pair<uint32_t, uint32_t> getWindow()
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return { getBase(), getSize() };
	}
};

class SyncedTokenWallet: S6U::TokenWallet
{
	mutable tbb::spin_mutex spinlock;

public:
	SyncedTokenWallet() {}
	
	SyncedTokenWallet(uint32_t base, uint32_t size)
		: TokenWallet(base, size) {}
	
	boost::optional<uint32_t> extract()
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return S6U::TokenWallet::extract();
	}
	
	void updateWindow(uint32_t newBase, uint32_t newSize)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return S6U::TokenWallet::updateWindow(newBase, newSize);
	}
	
	void updateWindow(const S6M::OptionSet *optionSet)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return S6U::TokenWallet::updateWindow(optionSet);
	}
	
	uint32_t remaining() const
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return S6U::TokenWallet::remaining();
	}
};

#endif // SYNCEDTOKENSTUFF_H
