#ifndef LISTENREACTOR_HH
#define LISTENREACTOR_HH

#include "reactor.hh"

class ListenReactor: public Reactor
{
protected:
	int listenFD;
	
	void processError(int err);
	
public:
	ListenReactor(int listenFD)
		: listenFD(listenFD) {}
	
	int getFD() const;
};

#endif // LISTENREACTOR_HH
