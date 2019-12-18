#include <system_error>
#include "proxyupstreamer.hh"
#include "connectproxydownstreamer.hh"

using namespace std;

ConnectProxyDownstreamer::ConnectProxyDownstreamer(ProxyUpstreamer *upstreamer, S6M::OperationReply *reply)
	: StreamReactor(upstreamer->getPoller(), SS_SENDING), upstreamer(upstreamer)
{
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
	
	srcSock.duplicate(upstreamer->getDstSock());
	dstSock.duplicate(upstreamer->getSrcSock());
}

void ConnectProxyDownstreamer::process(int fd, uint32_t events)
{
	upstreamer->getTimer()->refresh();
	StreamReactor::process(fd, events);
}

void ConnectProxyDownstreamer::deactivate()
{
	StreamReactor::deactivate();
	upstreamer->deactivate();
}
