#ifndef CLIENTSESSION_HH
#define CLIENTSESSION_HH

#include <boost/optional.hpp>
#include <vector>
#include "../authentication/syncedtokenstuff.h"

class ClientSession
{
	std::vector<uint8_t> id;
	boost::optional<SyncedTokenWallet> wallet;
	
public:
	ClientSession(const std::vector<uint8_t> &id, uint32_t winBase = 0, uint32_t winSize = 0)
		: id(id)
	{
		if (winSize > 0)
			wallet = SyncedTokenWallet(winBase, winSize);
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
