#ifndef CLIENTSESSION_HH
#define CLIENTSESSION_HH

#include <memory>
#include <vector>
#include <socks6util/socks6util.hh>

class ClientSession
{
	S6M::SessionID id;
	bool untrusted;
	std::unique_ptr<S6U::SyncedTokenWallet> wallet;
	
public:
	ClientSession(const S6M::SessionID &id, bool untrusted, std::pair<uint32_t, uint32_t> window = { 0, 0 })
		: id(id), untrusted(untrusted)
	{
		if (window.second > 0)
			wallet.reset(new S6U::SyncedTokenWallet(window));
	}

	const S6M::SessionID *getID() const
	{
		return &id;
	}

	bool isUntrusted() const
	{
		return untrusted;
	}
	
	void updateWallet(std::pair<uint32_t, uint32_t> window)
	{
		if (!wallet)
			return;
		if (window.second == 0)
			return;
		
		wallet->updateWindow(window);
	}
	
	std::optional<uint32_t> getToken()
	{
		if (!wallet)
			return {};
		return wallet->extract();
	}
};

#endif // CLIENTSESSION_HH
