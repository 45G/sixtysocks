#ifndef STICKREACTOR_HH
#define STICKREACTOR_HH

#include "reactor.hh"

class StickReactor: public Reactor
{
protected:
	UniqFD sock;

	StreamBuffer buf;

public:
	StickReactor(Poller *poller)
		: Reactor(poller) {}

	~StickReactor();

	void deactivate();
};

#endif // STICKREACTOR_HH
