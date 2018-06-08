#include <system_error>
#include "proxifierupstreamer.hh"
#include "proxifierdownstreamer.hh"

using namespace std;

ProxifierDownstreamer::ProxifierDownstreamer(ProxifierUpstreamer *upstreamer)
	: StreamReactor(-1, -1), owner(upstreamer->getOwner()), upstreamer(upstreamer), state(S_WAITING_FOR_AUTH_REP)
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
	upstreamer->use();
}

ProxifierDownstreamer::~ProxifierDownstreamer()
{
	if (upstreamer != NULL)
		upstreamer->unuse();
}

void ProxifierDownstreamer::process(Poller *poller, uint32_t events)
{
	(void)events;

	switch (state)
	{
	case S_WAITING_FOR_AUTH_REP:
	{
		break;
	}
	case S_WAITING_FOR_OP_REP:
	{
		break;
	}
	case S_STREAM:
		StreamReactor::process(poller, events);
	}
	}
}

int ProxifierDownstreamer::getFD() const
{
	//TODO
}
