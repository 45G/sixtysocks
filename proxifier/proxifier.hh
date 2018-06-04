#ifndef PROXIFIER_HH
#define PROXIFIER_HH

#include "../core/listenreactor.hh"

class Proxifier: public ListenReactor
{
	sockaddr_storage proxy;
	
public:
	Proxifier(const sockaddr_storage &proxy, int listenFD)
		: ListenReactor(listenFD), proxy(proxy) {}
	
	void process(Poller *poller, uint32_t events);
	
	~Proxifier();
};

#endif // PROXIFIER_HH
