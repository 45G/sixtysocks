#ifndef CLIENTSESSION_HH
#define CLIENTSESSION_HH

#include <memory>
#include <vector>
#include "../authentication/syncedtokenstuff.h"

class ClientSession
{
	std::vector<uint8_t> id;
	bool untrusted;
	std::unique_ptr<SyncedTokenWallet> wallet;
	
public:
	ClientSession(const std::vector<uint8_t> &id, bool untrusted, uint32_t winBase = 0, uint32_t winSize = 0)
		: id(id), untrusted(untrusted)
	{
		if (winSize > 0)
			wallet.reset(new SyncedTokenWallet(winBase, winSize));
	}

	const std::vector<uint8_t> *getID() const
	{
		return &id;
	}

	bool isUntrusted() const
	{
		return untrusted;
	}
	
	void updateWallet(uint32_t winBase, uint32_t winSize)
	{
		if (!wallet)
			return;
		if (winSize == 0)
			return;
		
		wallet->updateWindow(winBase, winSize);
	}
	
	boost::optional<uint32_t> getToken()
	{
		uint32_t ret;
		if (!wallet)
			return {};
		if (!wallet->extract(&ret))
			return {};
		
		return ret;
	}
};

#endif // CLIENTSESSION_HH
