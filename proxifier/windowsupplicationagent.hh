#ifndef WINDOWSUPPLICATIONAGENT_HH
#define WINDOWSUPPLICATIONAGENT_HH

#include "../core/reactor.hh"
#include "../core/streambuffer.hh"
#include "../core/uniqfd.hh"
#include "windowsupplicant.hh"

class Proxifier;

class WindowSupplicationAgent: public Reactor
{
	enum State
	{
		S_CONNECTING,
		S_SENDING_REQ,
		S_RECEIVING_AUTH_REP,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	UniqFD sock;
	
	StreamBuffer buf;
	
	State state;
	
	boost::shared_ptr<WindowSupplicant> supplicant;
	
public:
	WindowSupplicationAgent(Proxifier *proxifier, boost::shared_ptr<WindowSupplicant> supplicant);
	
	void process(int fd, uint32_t events);
	
	void deactivate();
	
	void start();
};

#endif // WINDOWSUPPLICATIONAGENT_HH
