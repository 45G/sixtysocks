#include <system_error>
#include "proxyupstreamer.hh"
#include "simpleproxydownstreamer.hh"

using namespace std;

SimpleProxyDownstreamer::SimpleProxyDownstreamer(ProxyUpstreamer *upstreamer, const S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), -1, -1, SS_WAITING_TO_SEND)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));

	dstFD.assign(dup(upstreamer->getDstFD()));
	if (dstFD < 0)
		throw system_error(errno, system_category());
}
