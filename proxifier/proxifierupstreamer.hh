#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include "../core/streamreactor.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_INIT_DATA,
		S_WAITING_TO_RECV,
		S_SENDING_REQ,
		S_WAITING_TO_SEND,
	};
	
	ssize_t reqBytesLeft;
	
	Proxifier *owner;
	ProxifierDownstreamer *downstreamer;
	
	State state;
	
public:
	ProxifierUpstreamer(Proxifier *owner, int srcFD)
		: StreamReactor(srcFD, -1), owner(owner), downstreamer(NULL), state(S_READING_INIT_DATA) {}
	
	void process(Poller *poller, uint32_t events);
	
	int getFD() const;
	
	Proxifier *getOwner() const
	{
		return owner;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
