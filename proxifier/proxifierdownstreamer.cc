#include <system_error>
#include "proxifierupstreamer.hh"
#include "proxifierdownstreamer.hh"

using namespace std;

ProxifierDownstreamer::ProxifierDownstreamer(ProxifierUpstreamer *upstreamer)
	: StreamReactor(-1, -1), owner(upstreamer->getOwner()), upstreamer(upstreamer)
{
	srcFD = dup(upstreamer->getDstFD());
	if (srcFD < 0)
		throw system_error(errno, system_category());
	
	dstFD = dup(upstreamer->getSrcFD());
	if (dstFD < 0)
	{
		// tolerable error
		close(srcFD);
		throw system_error(errno, system_category());
	}
}

void ProxifierDownstreamer::process(Poller *poller, uint32_t events)
{
	//TODO
}

int ProxifierDownstreamer::getFD() const
{
	//TODO
}
