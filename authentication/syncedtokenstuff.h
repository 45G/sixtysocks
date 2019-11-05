#ifndef SYNCEDTOKENSTUFF_H
#define SYNCEDTOKENSTUFF_H

#include <socks6util/idempotence.hh>
#include <tbb/spin_mutex.h>

class SyncedTokenBank: S6U::TokenBank
{
	tbb::spin_mutex spinlock;

public:
	using TokenBank::TokenBank;
	
	bool withdraw(uint32_t token)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return TokenBank::withdraw(token);
	}
	
	std::pair<uint32_t, uint32_t> getWindow()
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return TokenBank::getWindow();
	}
};

class SyncedTokenWallet: S6U::TokenWallet
{
	mutable tbb::spin_mutex spinlock;

public:
	using TokenWallet::TokenWallet;
	
	boost::optional<uint32_t> extract()
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return TokenWallet::extract();
	}
	
	void updateWindow(std::pair<uint32_t, uint32_t> window)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return TokenWallet::updateWindow(window);
	}
	
	void updateWindow(const S6M::OptionSet *optionSet)
	{
		tbb::spin_mutex::scoped_lock lock(spinlock);
		return S6U::TokenWallet::updateWindow(optionSet);
	}
};

#endif // SYNCEDTOKENSTUFF_H
