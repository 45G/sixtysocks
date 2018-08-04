#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "../core/sockio.hh"
#include "proxifier.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;

static const size_t HEADROOM = 512; //more than enough for any request

ProxifierUpstreamer::ProxifierUpstreamer(Proxifier *proxifier, int *pSrcFD, boost::shared_ptr<WindowSupplicant> supplicant)
	: StreamReactor(proxifier->getPoller(), SS_SENDING), proxifier(proxifier), state(S_CONNECTING), supplicant(supplicant)
{
	buf.makeHeadroom(HEADROOM);

	srcFD.assign(*pSrcFD);
	*pSrcFD = -1;
	
	dstFD.assign(socket(proxifier->getProxyAddr()->storage.ss_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (dstFD < 0)
		throw system_error(errno, system_category());
	
	int rc = S6U::Socket::getOriginalDestination(srcFD, &dest.storage);
	if (rc < 0)
		throw system_error(errno, system_category());
}

void ProxifierUpstreamer::start()
{
	/* read initial data opportunistically */
	ssize_t bytes = 0;
	try
	{
		bytes = sockFill(&srcFD, &buf);
	}
	catch (ReschedDisposition &) {}

	S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort(), 0);
	if (S6U::Socket::tfoAttempted(srcFD))
		req.getOptionSet()->setTFO();

	if (proxifier->getUsername()->length() > 0)
		req.getOptionSet()->setUsernamePassword(proxifier->getUsername(), proxifier->getPassword());

	/* check if idempotence is wanted */
	uint32_t polFlags = 0;
	if (buf.usedSize() == 0)
		polFlags |= S6U::TFOSafety::TFOS_NO_DATA;
	if (req.getOptionSet()->getTFO())
		polFlags |= S6U::TFOSafety::TFOS_TFO_SYN;
	if (!S6U::TFOSafety::tfoSafe(polFlags))
	{
		wallet = proxifier->getWallet();
		uint32_t token;

		if (wallet->extract(&token))
		{
			req.getOptionSet()->setToken(token);
			polFlags |= S6U::TFOSafety::TFOS_SPEND_TOKEN;
		}
	}

	if (supplicant.get() != NULL)
		supplicant->process(&req);

	uint8_t reqBuf[HEADROOM];
	S6M::ByteBuffer bb(reqBuf, sizeof(reqBuf));
	req.pack(&bb);
	buf.prepend(bb.getBuf(), bb.getUsed());

	/* connect */
	if (S6U::TFOSafety::tfoSafe(polFlags))
		bytes = sockSpillTFO(&dstFD, &buf, *proxifier->getProxyAddr());
	else
		bytes = connect(dstFD, &proxifier->getProxyAddr()->sockAddress, dest.size());
	if (bytes < 0 && errno != EINPROGRESS)
		throw system_error(errno, system_category());

	StreamReactor::start();
}

void ProxifierUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_CONNECTING:
	{
		poller->assign(new ProxifierDownstreamer(this));
		state = S_STREAM;
		
		streamState = buf.usedSize() > 0 ? SS_SENDING : SS_RECEIVING;
		
		if (buf.usedSize() > 0)
		{
			streamState = SS_SENDING;
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		}
		else
		{
			streamState = SS_RECEIVING;
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}
			
		break;
	}
		
	case S_STREAM:
		StreamReactor::process(fd, events);
		break;
	}
}
