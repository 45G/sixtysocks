#ifndef PROXIFIERACCEPTREACTOR_HH
#define PROXIFIERACCEPTREACTOR_HH

#include "reactor.hh"

class ProxifierAcceptReactor: public Reactor
{
public:
	ProxifierAcceptReactor(int fd);
	
	void process(Poller *poller, uint32_t events);
	uint32_t desiredEvents();
	
	~ProxifierAcceptReactor();
};

#endif // PROXIFIERACCEPTREACTOR_HH
