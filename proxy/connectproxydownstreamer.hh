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

	State state = S_INIT;

	boost::intrusive_ptr<ProxyUpstreamer> upstreamer;

public:
	ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply);

	void deactivate();
};

#endif // CONNECTPROXYDOWNSTREAMER_HH
