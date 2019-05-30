#ifndef PROXYSESSION_HH
#define PROXYSESSION_HH

#include <random>
#include "../authentication/syncedtokenstuff.h"
#include <boost/optional.hpp>

class ProxySession
{
	uint64_t id { ((uint64_t)rand()) | ((uint64_t)rand() << 32) };
	
	Spinlock bankCreationLock;
	boost::optional<SyncedTokenBank> tokenBank;
	
public:
	uint64_t getId() const
	{
		return id;
	}
	
	SyncedTokenBank *getTokenBank()
	{
		return tokenBank ? &tokenBank.get() : nullptr;
	}
	
	void makeBank(unsigned size)
	{
		std::lock_guard<Spinlock> scopedLock(bankCreationLock);
		
		if (tokenBank)
			return;
		tokenBank = SyncedTokenBank((uint32_t)rand(), size, 0, size / 2);
	}
};

#endif // PROXYSESSION_HH
