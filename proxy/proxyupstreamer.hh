#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <boost/intrusive_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include <boost/shared_ptr.hpp>
#include "core/streamreactor.hh"
#include "core/spinlock.hh"

class Proxy;
class ConnectProxyDownstreamer;
class AuthServer;

class ProxyUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_REQ,
		S_READING_INIT_DATA,
		S_AWAITING_AUTH,
		S_CONNECTING,
		S_STREAM,
	};

	volatile State state;
	volatile bool authenticated;
	
	boost::shared_ptr<S6M::Request> req;
	S6M::OptionSet replyOptions;

	boost::intrusive_ptr<ConnectProxyDownstreamer> downstreamer;
	
	AuthServer *authServer;
	Spinlock honorLock;

	void honorRequest();
	
public:
	ProxyUpstreamer(Proxy *owner, int srcFD);
	
	void process(int fd, uint32_t events);
	
	void authDone(SOCKS6TokenExpenditureCode expenditureCode);
};

#endif // PROXYUPSTREAMER_HH
