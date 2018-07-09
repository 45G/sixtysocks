#ifndef SIMPLEPROXYDOWNSTREAMER_HH
#define SIMPLEPROXYDOWNSTREAMER_HH

#include <boost/shared_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include "../core/streamreactor.hh"

class ProxyUpstreamer;

class SimpleProxyDownstreamer: public StreamReactor
{
public:
	SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const S6M::OperationReply *reply);

	void process(int fd, uint32_t events);
};

#endif // SIMPLEPROXYDOWNSTREAMER_HH
