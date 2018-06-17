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
	
	void process();
	
	virtual void setupReactor(int fd) = 0;
	
	void deactivate();

	~ListenReactor();
};

#endif // LISTENREACTOR_HH
