#ifndef PROXY_HH
#define PROXY_HH

#include <memory>
#include <unordered_map>
#include <string>
#include <socks6util/socks6util.hh>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include "../core/listenreactor.hh"
#include "../authentication/passwordchecker.hh"
#include "../authentication/syncedtokenstuff.h"

class Proxy: public ListenReactor
{
	std::unique_ptr<PasswordChecker> passwordChecker;
	
	std::unordered_map<std::string, std::unique_ptr<SyncedTokenBank> > banks;
	Spinlock bankLock;
	
	WOLFSSL_CTX *tlsCtx;

public:
	Proxy(Poller *poller, const S6U::SocketAddress &bindAddr, PasswordChecker *passwordChecker, WOLFSSL_CTX *tlsCtx)
		: ListenReactor(poller, bindAddr), passwordChecker(passwordChecker), tlsCtx(tlsCtx) {}
	
	void handleNewConnection(int fd);

	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
	
	SyncedTokenBank *createBank(const std::string &user, uint32_t size);
	
	SyncedTokenBank *getBank(const std::string &user);
};

#endif // PROXY_HH
