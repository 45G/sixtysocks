#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include <socks6util/socks6util.hh>
#include "../authentication/syncedtokenstuff.h"
#include "../core/streamreactor.hh"
#include "sessionsupplicant.hh"
#include "clientsession.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_CONNECTING,
		S_STREAM,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state = S_CONNECTING;
	
	std::shared_ptr<ClientSession> session;
	
	S6U::SocketAddress dest;
	
	std::shared_ptr<SessionSupplicant> sessionSupplicant;
	
public:
	ProxifierUpstreamer(Proxifier *proxifier, int *pSrcFD, TLSContext *clientCtx, std::shared_ptr<SessionSupplicant> sessionSupplicant);

	void start();
	
	void process(int fd, uint32_t events);
	
	Proxifier *getProxifier()
	{
		return proxifier.get();
	}
	
	std::shared_ptr<ClientSession> getSession() const
	{
		return session;
	}
	
	std::shared_ptr<SessionSupplicant> getSupplicant()
	{
		return sessionSupplicant;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
