#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <memory>
#include <boost/intrusive_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include "core/streamreactor.hh"
#include "../core/timer.hh"
#include "timeouts.hh"

class Proxy;
class ConnectProxyDownstreamer;
class AuthServer;

class ProxyUpstreamer: public StreamReactor
{
	const size_t MSS = 1460; //TODO: don't hardcode

	enum State
	{
		S_READING_REQ,
		S_READING_TFO_PAYLOAD,
		S_CONNECTING,
		//S_AWAINING_HUP,
		S_STREAM,
	};

	boost::intrusive_ptr<Proxy> proxy;
	
	std::atomic<State> state { S_READING_REQ };
	size_t tfoPayload = 0;
	
	std::shared_ptr<S6M::Request> request;
	S6M::OperationReply reply { SOCKS6_OPERATION_REPLY_FAILURE };

	boost::intrusive_ptr<ConnectProxyDownstreamer> downstreamer;
	
	AuthServer *authServer = nullptr;

	ReactorInactivityTimer timer { T_IDLE_CONNECTION, this };

	/* resolve state */
	S6M::Address addr;
	
	void addrFixupAndHonorRequest();
	
	void honorRequest();

	void honorConnect();

	void honorConnectStackOptions();
	
	void populateConnectStackOptions();
	
public:
	ProxyUpstreamer(Proxy *proxy, UniqFD &&srcFD);
	
	void start();
	
	void process(int fd, uint32_t events);
	
	void authDone()
	{
		addrFixupAndHonorRequest();
	}
	
	void resolvDone(std::optional<S6M::Address> resolved);

	std::shared_ptr<S6M::Request> getRequest() const
	{
		return request;
	}
	
	Proxy *getProxy()
	{
		return proxy.get();
	}
	
	ReactorInactivityTimer *getTimer()
	{
		return &timer;
	}
};

#endif // PROXYUPSTREAMER_HH
