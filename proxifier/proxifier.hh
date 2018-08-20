#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <socks6util/socks6util.hh>
#include "../core/tlscontext.hh"
#include "../authentication/syncedtokenstuff.h"
#include "../core/listenreactor.hh"
#include "../core/spinlock.hh"
#include "../core/tlssession.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	bool defer;

	const boost::shared_ptr<std::string> username;
	const boost::shared_ptr<std::string> password;
	
	bool idempotence;
	
	boost::shared_ptr<SyncedTokenWallet> wallet;
	Spinlock walletLock;
	Spinlock supplicationLock;

	TLSContext *clientCtx;
	TLSSession session;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, bool defer, const std::string &username, const std::string &passwordd, TLSContext *clientCtx);
	
	const S6U::SocketAddress *getProxyAddr() const
	{
		return &proxyAddr;
	}
	
	void start();

	void handleNewConnection(int fd);

	const boost::shared_ptr<std::string> getUsername() const
	{
		return username;
	}
	
	const boost::shared_ptr<std::string> getPassword() const
	{
		return password;
	}
	
	boost::shared_ptr<SyncedTokenWallet> getWallet()
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		return wallet;
	}
	
	void killWallet(boost::shared_ptr<SyncedTokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		if (wallet.get() == this->wallet.get())
			this->wallet = boost::shared_ptr<SyncedTokenWallet>(new SyncedTokenWallet());
	}
	
	void setWallet(boost::shared_ptr<SyncedTokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		this->wallet = wallet;
	}
	
	void supplicantDone()
	{
		supplicationLock.release();
	}
	
	TLSSession *getSession()
	{
		return &session;
	}
};

#endif // PROXIFIER_HH
