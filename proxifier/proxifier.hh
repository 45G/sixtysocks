#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <memory>
#include <socks6util/socks6util.hh>
#include "../core/tlscontext.hh"
#include "../authentication/syncedtokenstuff.h"
#include "../core/listenreactor.hh"
#include "../core/spinlock.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	bool defer;

	const std::string username;
	const std::string password;
	
	bool idempotence;
	
	std::shared_ptr<SyncedTokenWallet> wallet;
	Spinlock walletLock;
	Spinlock supplicationLock;

	TLSContext *clientCtx;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, bool defer, const std::string &username, const std::string &passwordd, TLSContext *clientCtx);
	
	const S6U::SocketAddress *getProxyAddr() const
	{
		return &proxyAddr;
	}
	
	void start();

	void handleNewConnection(int fd);

	const std::string *getUsername() const
	{
		return &username;
	}
	
	const std::string *getPassword() const
	{
		return &password;
	}
	
	std::shared_ptr<SyncedTokenWallet> getWallet()
	{
		std::lock_guard<Spinlock> lock(walletLock);
		
		return wallet;
	}
	
	void killWallet(std::shared_ptr<SyncedTokenWallet> wallet)
	{
		std::lock_guard<Spinlock> lock(walletLock);
		
		if (wallet.get() == this->wallet.get())
			this->wallet = std::shared_ptr<SyncedTokenWallet>(new SyncedTokenWallet());
	}
	
	void setWallet(std::shared_ptr<SyncedTokenWallet> wallet)
	{
		std::lock_guard<Spinlock> lock(walletLock);
		
		this->wallet = wallet;
	}
	
	void supplicantDone()
	{
		supplicationLock.unlock();
	}
};

#endif // PROXIFIER_HH
