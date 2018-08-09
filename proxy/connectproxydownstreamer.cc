#include <system_error>
#include "proxyupstreamer.hh"
#include "connectproxydownstreamer.hh"

using namespace std;
using namespace boost;

ConnectProxyDownstreamer::ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING), state(S_INIT), upstreamer(upstreamer)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
	
	srcSock.fd.assign(dup(upstreamer->getDstSock()->fd));
	if (srcSock.fd < 0)
		throw system_error(errno, system_category());

	dstSock.fd.assign(dup(upstreamer->getSrcSock()->fd));
	if (dstSock.fd < 0)
		throw system_error(errno, system_category());
}

void ConnectProxyDownstreamer::deactivate()
{
	StreamReactor::deactivate();
	upstreamer->deactivate();
}
