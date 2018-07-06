#ifndef PROXYDOWNSTREAMER_HH
#define PROXYDOWNSTREAMER_HH

#include <socks6msg/socks6msg.hh>
#include "../core/streamreactor.hh"

class ProxyUpstreamer;

class ProxyDownstreamer: public StreamReactor
{
	enum State
	{
		S_INIT,
		S_SENDING_AUTH_REP,
		S_SENDING_OP_REP,
	};

	State state;

public:
	ProxyDownstreamer(ProxyUpstreamer *upstreamer);

	void enqueue(const S6M::AuthenticationReply *authRep);

	void enqueue(S6M::OperationReply *opRep);
};

#endif // PROXYDOWNSTREAMER_HH
