#include <system_error>
#include "proxyupstreamer.hh"
#include "simpleproxydownstreamer.hh"

using namespace std;

SimpleProxyDownstreamer::SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));

	dstFD.assign(dup(upstreamer->getSrcFD()));
	if (dstFD < 0)
		throw system_error(errno, system_category());
}

SimpleProxyDownstreamer::SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const SOCKS6Version *version)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING)
{
	if (buf.availSize() > sizeof(*version))
		throw runtime_error("buffer too small");
	memcpy(buf.getTail(), version, sizeof(*version));
	buf.use(sizeof(*version));

	dstFD.assign(dup(upstreamer->getSrcFD()));
	if (dstFD < 0)
		throw system_error(errno, system_category());
}
