#ifndef LISTENREACTOR_HH
#define LISTENREACTOR_HH

#include "uniqfd.hh"
#include "reactor.hh"

class ListenReactor: public Reactor
{
protected:
	UniqFD listenFD;
	
public:
	ListenReactor(Poller *poller, const S6U::SocketAddress &bindAddr);
	
	void process(int fd, uint32_t events);
	
	virtual void handleNewConnection(int fd) = 0;
	
	void deactivate();

	void start();

	~ListenReactor();
};

#endif // LISTENREACTOR_HH
