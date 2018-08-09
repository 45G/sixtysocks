#ifndef WINDOWSUPPLICATIONAGENT_HH
#define WINDOWSUPPLICATIONAGENT_HH

#include "../core/stickreactor.hh"
#include "windowsupplicant.hh"

class Proxifier;

class WindowSupplicationAgent: public StickReactor
{
	enum State
	{
		S_CONNECTING,
		S_HANDSHAKING,
		S_SENDING_REQ,
		S_RECEIVING_AUTH_REP,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state;
	
	boost::shared_ptr<WindowSupplicant> supplicant;
	
public:
	WindowSupplicationAgent(Proxifier *proxifier, boost::shared_ptr<WindowSupplicant> supplicant);
	
	void process(int fd, uint32_t events);
	
	void start();
};

#endif // WINDOWSUPPLICATIONAGENT_HH
