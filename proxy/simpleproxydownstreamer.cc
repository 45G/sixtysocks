#include <system_error>
#include "proxyupstreamer.hh"
#include "simpleproxydownstreamer.hh"

using namespace std;

SimpleProxyDownstreamer::SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller())
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));

	dstSock.duplicate(upstreamer->getSrcSock());
}

SimpleProxyDownstreamer::SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const SOCKS6Version *version)
	: StreamReactor(upstreamer->getPoller())
{
	if (buf.availSize() < sizeof(*version))
		throw runtime_error("buffer too small");
	memcpy(buf.getTail(), version, sizeof(*version));
	buf.use(sizeof(*version));

	dstSock.duplicate(upstreamer->getSrcSock());
}
