#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <socks6util/socks6util.hh>
#include "../authentication/lockabletokenstuff.h"
#include "../core/listenreactor.hh"
#include "../core/spinlock.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	const boost::shared_ptr<std::string> username;
	const boost::shared_ptr<std::string> password;
	
	bool idempotence;
	
	boost::shared_ptr<LockableTokenWallet> wallet;
	Spinlock walletLock;
	Spinlock supplicationLock;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, const std::string &username = "", const std::string &password = "", bool idempotence = false);
	
	const S6U::SocketAddress *getProxyAddr() const
	{
		return &proxyAddr;
	}
	
	void start(bool defer);

	void handleNewConnection(int fd);

	const boost::shared_ptr<std::string> getUsername() const
	{
		return username;
	}
	
	const boost::shared_ptr<std::string> getPassword() const
	{
		return password;
	}
	
	boost::shared_ptr<LockableTokenWallet> getWallet()
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		return wallet;
	}
	
	void killWallet(boost::shared_ptr<LockableTokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		if (wallet.get() == this->wallet.get())
			wallet = boost::shared_ptr<LockableTokenWallet>(new LockableTokenWallet());
	}
	
	void setWallet(boost::shared_ptr<LockableTokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		this->wallet = wallet;
	}
	
	void supplicantDone()
	{
		supplicationLock.release();
	}
	
	bool idempotenceForTFO()
	{
		return idempotence;
	}
};

#endif // PROXIFIER_HH
