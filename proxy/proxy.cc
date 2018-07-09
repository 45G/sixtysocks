#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

void Proxy::handleNewConnection(int fd)
{
	ProxyUpstreamer *upstreamReactor = NULL;
	try
	{
		upstreamReactor = new ProxyUpstreamer(this, fd);
	}
	catch (...)
	{
		close(fd); // tolerable error
		return;
	}
	
	upstreamReactor->start(true);
}
