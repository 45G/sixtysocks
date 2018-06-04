#ifndef PROXIFERUPSTREAMREACTOR_HH
#define PROXIFERUPSTREAMREACTOR_HH

#include "streamreactor.hh"

class ProxiferUpstreamReactor: public StreamReactor
{
	enum State
	{
		S_READING_INIT_DATA,
		S_WAITING_FOR_DATA,
		S_CONNECTING,
		S_SENDING_REQ,
		S_WAITING_TO_SEND,
	};
	
	State state;

public:
	ProxiferUpstreamReactor(int srcFD);
	
	void process(Poller *poller, uint32_t events);
	
	int getFD() const;
};

#endif // PROXIFERUPSTREAMREACTOR_HH
