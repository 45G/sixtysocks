#include "../core/poller.hh"
#include "proxy.hh"

void Proxy::setupReactor(int fd)
{
	ProxifierUpstreamer *upstreamReactor = NULL;
	try
	{
		upstreamReactor = new ProxifierUpstreamer(this, fd);
	}
	catch (...)
	{
		close(fd); // tolerable error
		return;
	}
	
	poller->add(upstreamReactor, fd, Poller::IN_EVENTS);
}
