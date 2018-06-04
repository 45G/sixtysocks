#ifndef PROXIFIERACCEPTREACTOR_HH
#define PROXIFIERACCEPTREACTOR_HH

#include "../core/listenreactor.hh"

class ProxifierAcceptReactor: public ListenReactor
{
public:
	ProxifierAcceptReactor(int listenFD)
		: ProxifierAcceptReactor(listenFD) {}
	
	void process(Poller *poller, uint32_t events);
	
	~ProxifierAcceptReactor();
};

#endif // PROXIFIERACCEPTREACTOR_HH
