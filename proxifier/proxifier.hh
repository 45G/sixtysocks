#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"
#include "../core/spinlock.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	const boost::shared_ptr<std::string> username;
	const boost::shared_ptr<std::string> password;
	
	bool idempotence;
	
	boost::shared_ptr<S6U::TokenWallet> wallet;
	Spinlock walletLock;
	volatile bool supplicating;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, int listenFD, const std::string &username = "", const std::string &password = "", bool idempotence = false)
		: ListenReactor(poller, listenFD), proxyAddr(proxyAddr),
		  username(new std::string(username)), password(new std::string(password)),
		  idempotence(idempotence), wallet(new S6U::TokenWallet()), supplicating(false) {}
	
	const S6U::SocketAddress *getProxyAddr() const
	{
		return &proxyAddr;
	}
	
	void handleNewConnection(int fd);

	const boost::shared_ptr<std::string> getUsername() const
	{
		return username;
	}
	
	const boost::shared_ptr<std::string> getPassword() const
	{
		return password;
	}
	
	boost::shared_ptr<S6U::TokenWallet> getWallet()
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		return wallet;
	}
	
	void killWallet(boost::shared_ptr<S6U::TokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		if (wallet.get() == this->wallet.get())
			wallet = boost::shared_ptr<S6U::TokenWallet>(new S6U::TokenWallet());
	}
	
	void setWallet(boost::shared_ptr<S6U::TokenWallet> wallet)
	{
		ScopedSpinlock lock(&walletLock); (void)lock;
		
		this->wallet = wallet;
	}
	
	void supplicantDone()
	{
		supplicating = false;
	}
	
	bool idempotenceForTFO()
	{
		return idempotence;
	}
};

#endif // PROXIFIER_HH
