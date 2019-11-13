#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <memory>
#include <boost/intrusive_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include "core/streamreactor.hh"

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
	
	/* resolve state */
	S6M::Address addr;
	
	void honorRequest();

	void honorConnect();

	void honorConnectStackOptions();
	
public:
	ProxyUpstreamer(Proxy *proxy, UniqFD &&srcFD);
	
	void start();
	
	void process(int fd, uint32_t events);
	
	void authDone()
	{
		honorRequest();
	}

	std::shared_ptr<S6M::Request> getRequest() const
	{
		return request;
	}
	
	Proxy *getProxy()
	{
		return proxy.get();
	}
	
	class SimpleReplyException: public std::exception
	{
		SOCKS6OperationReplyCode code;
		
	public:
		SimpleReplyException(SOCKS6OperationReplyCode code)
			: code(code) {}
		
		SOCKS6OperationReplyCode getCode() const
		{
			return code;
		}
	};
};

#endif // PROXYUPSTREAMER_HH
