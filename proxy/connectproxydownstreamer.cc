#include <system_error>
#include "proxyupstreamer.hh"
#include "connectproxydownstreamer.hh"

using namespace std;
using namespace boost;

ConnectProxyDownstreamer::ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), -1, -1, SS_WAITING_TO_SEND), state(S_INIT)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
	
	srcFD = dup(upstreamer->getDstFD());
	if (srcFD < 0)
		throw system_error(errno, system_category());

	dstFD = dup(upstreamer->getSrcFD());
	if (dstFD < 0)
	{
		close(srcFD); // tolerable error
		throw system_error(errno, system_category());
	}
}
