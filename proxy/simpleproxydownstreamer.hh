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

	SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const SOCKS6Version *version);
};

#endif // SIMPLEPROXYDOWNSTREAMER_HH
