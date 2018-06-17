#include "proxy.hh"
#include "proxyupstreamer.hh"

ProxyUpstreamer::ProxyUpstreamer(Proxy *owner, int srcFD)
	: StreamReactor(owner->getPoller(), srcFD, -1), state(S_READING_REQ) {}

void ProxyUpstreamer::process(int fd, uint32_t events)
{
	//TODO
}
