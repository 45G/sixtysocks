#ifndef SERVERSESSION_HH
#define SERVERSESSION_HH

#include <memory>
#include <random>
#include <socks6util/socks6util.hh>

class ServerSession
{
	uint64_t id { ((uint64_t)rand()) | ((uint64_t)rand() << 32) };
	
	tbb::spin_mutex bankCreationLock;
	std::unique_ptr<S6U::SyncedTokenBank> tokenBank;
	
public:
	uint64_t getID() const
	{
		return id;
	}
	
	S6U::SyncedTokenBank *getTokenBank()
	{
		return tokenBank.get();
	}
	
	void makeBank(unsigned size)
	{
		tbb::spin_mutex::scoped_lock scopedLock(bankCreationLock);
		
		if (tokenBank)
			return;
		if (size == 0)
			return;

		tokenBank.reset(new S6U::SyncedTokenBank({ (uint32_t)rand(), size }, 0, size / 2));
	}
};

#endif // SERVERSESSION_HH
