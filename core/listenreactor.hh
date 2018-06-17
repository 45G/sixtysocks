#ifndef LISTENREACTOR_HH
#define LISTENREACTOR_HH

#include "reactor.hh"

class ListenReactor: public Reactor
{
protected:
	int listenFD;
	
public:
	ListenReactor(Poller *poller, int listenFD)
		: Reactor(poller), listenFD(listenFD) {}
	
	void process(int fd, uint32_t events);
	
	virtual void handleNewConnection(int fd) = 0;
	
	void deactivate();

	~ListenReactor();
};

#endif // LISTENREACTOR_HH
