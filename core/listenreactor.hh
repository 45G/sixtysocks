#ifndef LISTENREACTOR_HH
#define LISTENREACTOR_HH

#include "fdwrapper.hh"
#include "reactor.hh"

class ListenReactor: public Reactor
{
protected:
	FDWrapper listenFD;
	
	void processError(int err);
	
public:
	ListenReactor(Poller *poller, int listenFD)
		: Reactor(poller), listenFD(listenFD) {}

	~ListenReactor();
};

#endif // LISTENREACTOR_HH
