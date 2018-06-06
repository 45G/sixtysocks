#ifndef PROXIFIERDOWNSTREAMER_HH
#define PROXIFIERDOWNSTREAMER_HH

#include "../core/streamreactor.hh"
#include "proxifier.hh"

class ProxifierUpstreamer;

class ProxifierDownstreamer : public StreamReactor
{
	enum State
	{
		S_WAITING_FOR_CONNECT,
		S_WAITING_FOR_DATA,
		S_CONNECTING,
		S_SENDING_REQ,
		S_WAITING_TO_SEND,
	};
	
	Proxifier *owner;
	ProxifierUpstreamer *upstreamer;
public:
	ProxifierDownstreamer(ProxifierUpstreamer *upstreamer);
	
	void process(Poller *poller, uint32_t events);
	
	int getFD() const;
};

#endif // PROXIFIERDOWNSTREAMER_HH
