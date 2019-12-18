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
#include "serversession.hh"
#include "resolver.hh"
#include "../core/timeoutreactor.hh"
#include "timeouts.hh"

class Proxy: public ListenReactor
{
	boost::intrusive_ptr<Resolver> resolver = new Resolver(poller);
	
	std::unique_ptr<PasswordChecker> passwordChecker;
	
	tbb::concurrent_hash_map<uint64_t, std::shared_ptr<ServerSession>> sessions;
	std::unordered_map<std::string, std::unique_ptr<S6U::SyncedTokenBank>> banks;
	tbb::spin_mutex bankLock;
	
	TLSContext *serverCtx;

	boost::intrusive_ptr<TimeoutReactor> timeoutReactor { new TimeoutReactor(poller, { T_IDLE_CONNECTION }) };

public:
	static const std::set<uint16_t> DEFAULT_SERVICES;

	Proxy(Poller *poller, const S6U::SocketAddress &bindAddr, PasswordChecker *passwordChecker, TLSContext *serverCtx)
		: ListenReactor(poller, bindAddr), passwordChecker(passwordChecker), serverCtx(serverCtx) {}

	void start();
	
	void handleNewConnection(int fd);

	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
	
	std::shared_ptr<ServerSession> spawnSession();
	
	std::shared_ptr<ServerSession> getSession(uint64_t id);
	
	S6U::SyncedTokenBank *createBank(const std::string &user, uint32_t size);
	
	S6U::SyncedTokenBank *getBank(const std::string &user);
	
	TLSContext *getServerCtx() const
	{
		return serverCtx;
	}
	
	Resolver *getResolver()
	{
		return resolver.get();
	}

	TimeoutReactor *getTimeoutReactor() const
	{
		return timeoutReactor.get();
	}
};

#endif // PROXY_HH
