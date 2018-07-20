#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxyAddr;

	const boost::shared_ptr<std::string> username;
	const boost::shared_ptr<std::string> password;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, int listenFD, const std::string &username = "", const std::string &password = "")
		: ListenReactor(poller, listenFD), proxyAddr(proxyAddr), username(new std::string(username)), password(new std::string(password)) {}
	
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
};

#endif // PROXIFIER_HH
