#ifndef PROXIFIERACCEPTFSM_HH
#define PROXIFIERACCEPTFSM_HH

#include "fsm.hh"

class ProxifierAcceptFSM: public FSM
{
public:
	ProxifierAcceptFSM(int fd);
	
	void process(Poller *poller, uint32_t events);
	uint32_t desiredEvents();
	
	~ProxifierAcceptFSM();
};

#endif // PROXIFIERACCEPTFSM_HH
