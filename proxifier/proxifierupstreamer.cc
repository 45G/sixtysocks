#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "proxifier.hh"
#include "proxifiertfopolicy.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;

ProxifierUpstreamer::ProxifierUpstreamer(Proxifier *owner, int srcFD)
	: StreamReactor(owner->getPoller(), srcFD, -1), owner(owner), state(S_READING_INIT_DATA) {}

void ProxifierUpstreamer::process()
{
	if (!active)
		return;
	
	switch (state)
	{
	case S_READING_INIT_DATA:
	{
		S6U::SocketAddress dest;
		S6U::Socket::getOriginalDestination(srcFD, &dest.storage);
		
		S6M::OptionSet opts(S6M::OptionSet::M_REQ);
		if (S6U::Socket::tfoAttempted(srcFD))
			opts.setTFO();
		S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort(), opts, 0);
		S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
		req.pack(&bb);
		buf.use(bb.getUsed());
		reqBytesLeft = bb.getUsed();
		
		ssize_t bytes = fill(srcFD);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}
		
		dstFD = socket(owner->getProxy()->storage.ss_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
		if (dstFD < 0)
			throw system_error(errno, system_category());
		
		state = S_SENDING_REQ;
		
		/* check if TFO is wanted */
		uint32_t polFlags = 0;
		if (bytes > 0)
			polFlags |= ProxifierTFOPolicy::F_GOT_DATA;
		if (opts.getTFO())
			polFlags |= ProxifierTFOPolicy::F_TFO_SYN;
		if (ProxifierTFOPolicy::tfoPermitted(polFlags))
		{
			bytes = spillTFO(dstFD, *owner->getProxy());
			if (bytes < 0)
			{
				if (errno != EINPROGRESS)
					throw system_error(errno, system_category());
				bytes = 0;
			}
			reqBytesLeft -= bytes;
		}
		else
		{
			int rc = connect(dstFD, &owner->getProxy()->sockAddress, dest.size());
			if (rc < 0)
			{
				if (errno != EINPROGRESS)
					throw system_error(errno, system_category());
			}
			bytes = 0;
		}
		
		if (reqBytesLeft < 0)
		{
			ProxifierDownstreamer *downstreamer = new ProxifierDownstreamer(this);
			poller->add(downstreamer, downstreamer->getSrcFD(), Poller::IN_EVENTS);
			state = S_STREAM;
			streamState = buf.usedSize() > 0 ? SS_WAITING_TO_SEND : SS_WAITING_TO_RECV;
		}
			
		if (state == S_SENDING_REQ || (state == S_STREAM && streamState == SS_WAITING_TO_SEND))
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		else
			poller->add(this, srcFD, Poller::IN_EVENTS);
		
		break;
	}
	case S_SENDING_REQ:
	{
		ssize_t bytes = spill(dstFD);
		if (bytes == 0)
			return;
		if (bytes < 0)
		{
			if (errno != EINPROGRESS)
				throw system_error(errno, system_category());
		}
		reqBytesLeft -= bytes;
		
		if (reqBytesLeft < 0)
		{
			ProxifierDownstreamer *downstreamer = new ProxifierDownstreamer(this);
			poller->add(downstreamer, downstreamer->getSrcFD(), Poller::IN_EVENTS);
			state = S_STREAM;
			streamState = buf.usedSize() > 0 ? SS_WAITING_TO_SEND : SS_WAITING_TO_RECV;
		}
			
		if (state == S_SENDING_REQ || (state == S_STREAM && streamState == SS_WAITING_TO_SEND))
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		else
			poller->add(this, srcFD, Poller::IN_EVENTS);
		
		break;
	}
	case S_STREAM:
		StreamReactor::process(poller);
		break;
	}
}
