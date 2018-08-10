#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "proxifier.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;

static const size_t HEADROOM = 512; //more than enough for any request

ProxifierUpstreamer::ProxifierUpstreamer(Proxifier *proxifier, int *pSrcFD, TLSContext *clientCtx, boost::shared_ptr<WindowSupplicant> supplicant)
	: StreamReactor(proxifier->getPoller(), SS_SENDING), proxifier(proxifier), state(S_CONNECTING), supplicant(supplicant)
{
	buf.makeHeadroom(HEADROOM);

	srcSock.fd.assign(*pSrcFD);
	*pSrcFD = -1;
	
	dstSock.fd.assign(socket(proxifier->getProxyAddr()->storage.ss_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (dstSock.fd < 0)
		throw system_error(errno, system_category());
	if (clientCtx != NULL)
		dstSock.tls = new TLS(clientCtx, dstSock.fd);
	
	int rc = S6U::Socket::getOriginalDestination(srcSock.fd, &dest.storage);
	if (rc < 0)
		throw system_error(errno, system_category());
}

void ProxifierUpstreamer::start()
{
	try
	{
		srcSock.serverHandshake(&buf);
	}
	catch (exception &)
	{
		throw runtime_error("Unexpected exception in null handshake");
	}
	
	/* read initial data opportunistically */
	try
	{
		srcSock.sockRecv(&buf);
	}
	catch (RescheduleException &) {}

	S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort(), 0);
	if (S6U::Socket::tfoAttempted(srcSock.fd))
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
	dstSock.sockConnect(*proxifier->getProxyAddr(), &buf, S6U::TFOSafety::tfoSafe(polFlags), polFlags & S6U::TFOSafety::TFOS_SPEND_TOKEN);

	poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
}

void ProxifierUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_CONNECTING:
	{
		state = S_HANDSHAKING;
		[[fallthrough]];
	}
	case S_HANDSHAKING:
	{
		dstSock.clientHandshake();

		poller->assign(new ProxifierDownstreamer(this));

		state = S_STREAM;
		streamState = buf.usedSize() > 0 ? SS_SENDING : SS_RECEIVING;
		
		if (buf.usedSize() > 0)
			streamState = SS_SENDING;
		else
			streamState = SS_RECEIVING;
		[[fallthrough]];
	}
		
	case S_STREAM:
		StreamReactor::process(fd, events);
		break;
	}
}
