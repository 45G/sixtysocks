#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <memory>
#include <socks6util/socks6util.hh>
#include "../core/tlscontext.hh"
#include "../authentication/syncedtokenstuff.h"
#include "../core/listenreactor.hh"
#include "../core/spinlock.hh"
#include "clientsession.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	bool defer;

	const std::string username;
	const std::string password;
	
	bool idempotence;
	
	std::shared_ptr<ClientSession> session;
	Spinlock sessionLock;
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
	
	std::shared_ptr<ClientSession> getSession()
	{
		std::lock_guard<Spinlock> lock(sessionLock);
		
		return session;
	}
	
	void killSession(std::shared_ptr<ClientSession> session)
	{
		std::lock_guard<Spinlock> lock(sessionLock);
		
		if (session.get() == this->session.get())
			this->session.reset();
	}
	
	void setSession(std::shared_ptr<ClientSession> session)
	{
		std::lock_guard<Spinlock> lock(sessionLock);
		
		this->session = session;
	}
	
	void supplicantDone()
	{
		supplicationLock.unlock();
	}
};

#endif // PROXIFIER_HH
