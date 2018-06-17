#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"

class Proxifier: public ListenReactor
{
	S6U::SocketAddress proxy;
	
public:
	Proxifier(Poller *poller, const S6U::SocketAddress &proxy, int listenFD)
		: ListenReactor(poller, listenFD), proxy(proxy) {}
	
	const S6U::SocketAddress *getProxy() const
	{
		return &proxy;
	}
	
	void setupReactor(int fd);
	
	~Proxifier();
};

#endif // PROXIFIER_HH
