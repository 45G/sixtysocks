#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <boost/intrusive_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include <boost/shared_ptr.hpp>
#include "core/streamreactor.hh"

class Proxy;
class ConnectProxyDownstreamer;

class ProxyUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_REQ,
		S_HONORING_REQ,
		S_SENDING_OP_REP,
		S_STREAM,
	};

	State state;
	boost::shared_ptr<S6M::Request> req;

	boost::intrusive_ptr<ConnectProxyDownstreamer> downstreamer;

	void authenticate();
	
public:
	ProxyUpstreamer(Proxy *owner, int srcFD);
	
	void process(int fd, uint32_t events);
};

#endif // PROXYUPSTREAMER_HH
