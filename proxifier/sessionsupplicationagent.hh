#ifndef SESSIONSUPPLICATIONAGENT_HH
#define SESSIONSUPPLICATIONAGENT_HH

#include "../core/stickreactor.hh"
#include "sessionsupplicant.hh"

class Proxifier;

class SessionSupplicationAgent: public StickReactor
{
	enum State
	{
		S_CONNECTING,
		S_HANDSHAKING,
		S_SENDING_REQ,
		S_RECEIVING_AUTH_REP,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state = S_CONNECTING;
	
	std::shared_ptr<SessionSupplicant> supplicant;

	TLSContext *clientCtx;
	
public:
	SessionSupplicationAgent(Proxifier *proxifier, std::shared_ptr<SessionSupplicant> supplicant, TLSContext *clientCtx);
	
	void process(int fd, uint32_t events);
	
	void start();
};

#endif // SESSIONSUPPLICATIONAGENT_HH
