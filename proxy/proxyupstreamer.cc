#include <system_error>
#include "../core/poller.hh"
#include "proxy.hh"
#include "proxyupstreamer.hh"

using namespace std;

ProxyUpstreamer::ProxyUpstreamer(Proxy *owner, int srcFD)
	: StreamReactor(owner->getPoller(), srcFD, -1), state(S_READING_REQ) {}

void ProxyUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_READING_REQ:
	{
		ssize_t bytes = fill(srcFD);
		if (bytes == 0)
			return;
		if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			throw system_error(errno, system_category());

		S6M bb(buf.getHead(), buf.usedSize());
		try
		{
			req = new S6M::Request(&bb);


		}
		catch (S6M::EndOfBufferException)
		{
			poller->add(this, srcFD, Poller::IN_EVENTS);
			return;
		}

		break;
	}
	case S_HONORING_REQ:
	{
		//TODO
		break;
	}
	case S_STREAM:
	{
		StreamReactor::process(fd, events);
		break;
	}
	}
}
