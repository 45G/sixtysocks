#ifndef PROXY_HH
#define PROXY_HH

#include <memory>
#include <unordered_map>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <socks6util/socks6util.hh>
#include "../tls/tlscontext.hh"
#include "../core/listenreactor.hh"
#include "../authentication/passwordchecker.hh"
#include "../authentication/syncedtokenstuff.h"
#include "serversession.hh"

class Proxy: public ListenReactor
{
	std::unique_ptr<PasswordChecker> passwordChecker;
	
	tbb::concurrent_hash_map<uint64_t, std::shared_ptr<ServerSession>> sessions;
	std::unordered_map<std::string, std::unique_ptr<SyncedTokenBank>> banks;
	tbb::spin_mutex bankLock;
	
	TLSContext *serverCtx;

public:
	Proxy(Poller *poller, const S6U::SocketAddress &bindAddr, PasswordChecker *passwordChecker, TLSContext *serverCtx)
		: ListenReactor(poller, bindAddr), passwordChecker(passwordChecker), serverCtx(serverCtx) {}
	
	void handleNewConnection(int fd);

	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
	
	std::shared_ptr<ServerSession> spawnSession();
	
	std::shared_ptr<ServerSession> getSession(uint64_t id);
	
	SyncedTokenBank *createBank(const std::string &user, uint32_t size);
	
	SyncedTokenBank *getBank(const std::string &user);
	
	TLSContext *getServerCtx() const
	{
		return serverCtx;
	}
};

#endif // PROXY_HH
