#ifndef PROXIFIERACCEPTREACTOR_HH
#define PROXIFIERACCEPTREACTOR_HH

#include "reactor.hh"

class ProxifierAcceptFSM: public Reactor
{
public:
	ProxifierAcceptFSM(int fd);
	
	void process(Poller *poller, uint32_t events);
	uint32_t desiredEvents();
	
	~ProxifierAcceptFSM();
};

#endif // PROXIFIERACCEPTREACTOR_HH
