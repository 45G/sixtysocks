#ifndef CONNECTPROXYDOWNSTREAMER_HH
#define CONNECTPROXYDOWNSTREAMER_HH

#include <socks6msg/socks6msg.hh>
#include "../core/streamreactor.hh"

class ProxyUpstreamer;

class ConnectProxyDownstreamer: public StreamReactor
{
	enum State
	{
		S_INIT,
		S_SENDING_AUTH_REP,
		S_SENDING_OP_REP,
	};

	State state;

public:
	ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply);
};

#endif // CONNECTPROXYDOWNSTREAMER_HH
