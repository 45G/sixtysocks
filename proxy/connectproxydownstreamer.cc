#include <system_error>
#include "proxyupstreamer.hh"
#include "connectproxydownstreamer.hh"

using namespace std;
using namespace boost;

ConnectProxyDownstreamer::ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING), state(S_INIT), upstreamer(upstreamer)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
	
	srcSock.duplicate(upstreamer->getDstSock());
	dstSock.duplicate(upstreamer->getSrcSock());
}

void ConnectProxyDownstreamer::deactivate()
{
	StreamReactor::deactivate();
	upstreamer->deactivate();
}
