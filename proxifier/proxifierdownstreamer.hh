#ifndef PROXIFIERDOWNSTREAMER_HH
#define PROXIFIERDOWNSTREAMER_HH

#include "../core/streamreactor.hh"
#include "proxifier.hh"

class ProxifierUpstreamer;

class ProxifierDownstreamer: public StreamReactor
{
	enum State
	{
		S_WAITING_FOR_AUTH_REP,
		S_WAITING_FOR_OP_REP,
		S_STREAM,
	};
	
	boost::intrusive_ptr<Proxifier> owner;
	boost::intrusive_ptr<ProxifierUpstreamer> upstreamer;

	State state;

public:
	ProxifierDownstreamer(ProxifierUpstreamer *upstreamer);
	
	void process(int fd, uint32_t events);
};

#endif // PROXIFIERDOWNSTREAMER_HH
