#include <system_error>
#include "proxyupstreamer.hh"
#include "connectproxydownstreamer.hh"

using namespace std;
using namespace boost;

ConnectProxyDownstreamer::ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING), state(S_INIT), upstreamer(upstreamer)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
	
	srcFD.assign(dup(*upstreamer->getDstFD()));
	if (srcFD < 0)
		throw system_error(errno, system_category());

	dstFD.assign(dup(*upstreamer->getSrcFD()));
	if (dstFD < 0)
		throw system_error(errno, system_category());
}

void ConnectProxyDownstreamer::deactivate()
{
	StreamReactor::deactivate();
	upstreamer->deactivate();
}
