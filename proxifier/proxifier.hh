#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;
	const std::string username;
	const std::string password;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, int listenFD, const std::string &username = "", const std::string &password = "")
		: ListenReactor(poller, listenFD), proxyAddr(proxyAddr), username(username), password(password) {}
	
	const S6U::SocketAddress *getProxy() const
	{
		return &proxyAddr;
	}
	
	void handleNewConnection(int fd);

	const std::string *getUsername() const
	{
		return &username;
	}
	const std::string *getPassword() const
	{
		return &password;
	}
};

#endif // PROXIFIER_HH
