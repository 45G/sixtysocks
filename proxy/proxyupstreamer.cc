#include <system_error>
#include "../core/poller.hh"
#include "proxy.hh"
#include "../authentication/noauthserver.hh"
#include "proxydownstreamer.hh"
#include "proxyupstreamer.hh"

using namespace std;
using namespace boost;

void ProxyUpstreamer::authenticate()
{
	//TODO: authentication policy
	//TODO: fast path (use return value; return bool)
	intrusive_ptr<NoAuthServer> noAuth = new NoAuthServer(this);
	noAuth->resume();
}

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

		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			req = boost::shared_ptr<S6M::Request>(new S6M::Request(&bb));
			authenticate();
			state = S_HONORING_REQ;
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
		switch (req->getCommandCode()) {
		default:
			S6M::OperationReply rep(SOCKS6_OPERATION_REPLY_CMD_NOT_SUPPORTED, S6M::Address({ .saddr = 0 }), 0,  S6M::OptionSet(S6M::OptionSet::M_OP_REP));
			break;
		}
		break;
	}
	case S_STREAM:
	{
		StreamReactor::process(fd, events);
		break;
	}
	}
}
