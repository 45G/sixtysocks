#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "proxifier.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;

ProxifierUpstreamer::ProxifierUpstreamer(Proxifier *proxifier, int srcFD, bool supplicate)
	: StreamReactor(proxifier->getPoller(), srcFD, -1, SS_WAITING_TO_SEND), proxifier(proxifier), state(S_CONNECTING), supplicating(supplicate)
{
	dstFD = socket(proxifier->getProxyAddr()->storage.ss_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (dstFD < 0)
		throw system_error(errno, system_category());
	
	S6U::Socket::getOriginalDestination(srcFD, &dest.storage);
	
	S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort(), 0);
	if (S6U::Socket::tfoAttempted(srcFD))
		req.getOptionSet()->setTFO();
	if (proxifier->getUsername()->length() > 0)
		req.getOptionSet()->setUsernamePassword(proxifier->getUsername(), proxifier->getPassword());
	
	/* check if idempotence is wanted */
	uint32_t polFlags = 0;
	if (S6U::Socket::pendingRecv(srcFD) <= 0)
		polFlags |= S6U::TFOSafety::TFOS_NO_DATA;
	if (req.getOptionSet()->getTFO())
		polFlags |= S6U::TFOSafety::TFOS_TFO_SYN;
	if (!S6U::TFOSafety::tfoSafe(polFlags) && proxifier->idempotenceForTFO())
	{
		wallet = proxifier->getWallet();
		uint32_t token;
		
		if (wallet.get() != NULL && wallet->extract(&token))
		{
			req.getOptionSet()->setToken(token);
			polFlags |= S6U::TFOSafety::TFOS_SPEND_TOKEN;
		}
	}
	
	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
	req.pack(&bb);
	buf.use(bb.getUsed());
	
	/* read initial data opportunistically */
	if (!(polFlags & S6U::TFOSafety::TFOS_NO_DATA))
	{
		ssize_t bytes = fill(srcFD);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}
	}
	
	/* connect */
	{
		ssize_t bytes;
		
		/* check if TFO is wanted */
		if (S6U::TFOSafety::tfoSafe(polFlags))
		{
			bytes = spillTFO(dstFD, *proxifier->getProxyAddr());
			if (bytes < 0)
			{
				if (errno != EINPROGRESS)
					throw system_error(errno, system_category());
			}
		}
		else
		{
			int rc = connect(dstFD, &proxifier->getProxyAddr()->sockAddress, dest.size());
			if (rc < 0)
			{
				if (errno != EINPROGRESS)
					throw system_error(errno, system_category());
			}
		}
	}
}

ProxifierUpstreamer::~ProxifierUpstreamer()
{
	if (supplicating)
		proxifier->supplicantDone();
}

void ProxifierUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_CONNECTING:
	{
		ProxifierDownstreamer *downstreamer = new ProxifierDownstreamer(this);
		downstreamer->start();
		state = S_STREAM;
		
		streamState = buf.usedSize() > 0 ? SS_WAITING_TO_SEND : SS_WAITING_TO_RECV;
		
		if (buf.usedSize() > 0)
		{
			streamState = SS_WAITING_TO_SEND;
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		}
		else
		{
			streamState = SS_WAITING_TO_RECV;
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}
			
		break;
	}
		
	case S_STREAM:
		StreamReactor::process(fd, events);
		break;
	}
}
