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

	boost::intrusive_ptr<Proxy> proxy;
	
	volatile State state;
	volatile bool authenticated;
	
	boost::shared_ptr<S6M::Request> request;
	S6M::OptionSet replyOptions;

	boost::intrusive_ptr<ConnectProxyDownstreamer> downstreamer;
	
	AuthServer *authServer;
	Spinlock honorLock;
	
	bool mustFail;

	void honorRequest();
	
public:
	ProxyUpstreamer(Proxy *proxy, int srcFD);
	
	void process(int fd, uint32_t events);
	
	void authDone(SOCKS6TokenExpenditureCode expenditureCode);

	boost::shared_ptr<S6M::Request> getRequest() const
	{
		return request;
	}
	
	Proxy *getProxy()
	{
		return proxy.get();
	}
	
	void fail()
	{
		mustFail = true;
	}
};

#endif // PROXYUPSTREAMER_HH
