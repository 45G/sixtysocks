#ifndef SUPPLICATIONAGENT_HH
#define SUPPLICATIONAGENT_HH

#include "../core/reactor.hh"
#include "../core/streambuffer.hh"
#include "../core/uniqfd.hh"
#include "windowsupplicant.hh"

class Proxifier;

class SupplicationAgent: public Reactor
{
	enum State
	{
		S_SENDING_REQ,
		S_RECEIVING_AUTH_REP,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	int fd;
	
	StreamBuffer buf;
	
	State state;
	
	WindowSupplicant supplicant;
	
public:
	SupplicationAgent(Proxifier *proxifier);
	
	void process(int fd, uint32_t events);
	
	void deactivate();
	
	void start(bool defer);
};

#endif // SUPPLICATIONAGENT_HH
