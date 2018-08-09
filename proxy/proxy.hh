#ifndef PROXY_HH
#define PROXY_HH

#include <memory>
#include <unordered_map>
#include <string>
#include <socks6util/socks6util.hh>
#include "core/tlscontext.hh"
#include "../core/listenreactor.hh"
#include "../authentication/passwordchecker.hh"
#include "../authentication/syncedtokenstuff.h"

class Proxy: public ListenReactor
{
	std::unique_ptr<PasswordChecker> passwordChecker;
	
	std::unordered_map<std::string, std::unique_ptr<SyncedTokenBank> > banks;
	Spinlock bankLock;
	
	TLSContext *serverCtx;

public:
	Proxy(Poller *poller, const S6U::SocketAddress &bindAddr, PasswordChecker *passwordChecker, TLSContext *serverCtx)
		: ListenReactor(poller, bindAddr), passwordChecker(passwordChecker), serverCtx(serverCtx) {}
	
	void handleNewConnection(int fd);

	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
	
	SyncedTokenBank *createBank(const std::string &user, uint32_t size);
	
	SyncedTokenBank *getBank(const std::string &user);
};

#endif // PROXY_HH
