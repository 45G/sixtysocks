#ifndef PROXY_HH
#define PROXY_HH

#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"

class Proxy: public ListenReactor
{
public:
	Proxy(Poller *poller, int listenFD)
		: ListenReactor(poller, listenFD) {}
	
	void handleNewConnection(int fd);
};

#endif // PROXY_HH
