#ifndef PROXIFERUPSTREAMER_HH
#define PROXIFERUPSTREAMER_HH

#include "../core/streamreactor.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxiferUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_INIT_DATA,
		S_WAITING_FOR_DATA,
		S_CONNECTING,
		S_SENDING_REQ,
		S_WAITING_TO_SEND,
	};
	
	Proxifier *owner;
	ProxifierDownstreamer *peer;
	
	State state;

public:
	ProxiferUpstreamer(Proxifier *owner, int srcFD)
		: StreamReactor(srcFD, -1), owner(owner), peer(NULL), state(S_READING_INIT_DATA) {}
	
	void process(Poller *poller, uint32_t events);
	
	int getFD() const;
};

#endif // PROXIFERUPSTREAMER_HH
