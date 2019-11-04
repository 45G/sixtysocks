#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "proxifier.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;
using namespace boost;

static const size_t HEADROOM = 512; //more than enough for any request

ProxifierUpstreamer::ProxifierUpstreamer(Proxifier *proxifier, UniqFD &&srcFD, std::shared_ptr<SessionSupplicant> sessionSupplicant)
	: StreamReactor(proxifier->getPoller(), SS_SENDING), proxifier(proxifier), session(proxifier->getSession()), sessionSupplicant(sessionSupplicant)
{
	buf.makeHeadroom(HEADROOM);

	srcSock.fd = UniqRecvFD(srcFD);
	
	dstSock.fd.assign(socket(proxifier->getProxyAddr()->storage.ss_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (dstSock.fd < 0)
		throw system_error(errno, system_category());
	
	TLSContext *clientCtx = proxifier->getClientCtx();
	if (clientCtx)
		dstSock.tls = new TLS(clientCtx, dstSock.fd);
	
	int rc = S6U::Socket::getOriginalDestination(srcSock.fd, &dest.storage);
	if (rc < 0)
		throw system_error(errno, system_category());
}

void ProxifierUpstreamer::start()
{
	/* read initial data opportunistically */
	try
	{
		srcSock.sockRecv(&buf);
	}
	catch (RescheduleException &) {}

	S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort());

	ssize_t tfoPayload = S6U::Socket::tfoPayloadSize(srcSock.fd);
	if (tfoPayload > 0)
		req.options.stack.tfo.set(SOCKS6_STACK_LEG_PROXY_REMOTE, tfoPayload);

	bool authenticate = proxifier->getUsername()->length() > 0;
	if (session)
	{
		req.options.session.setID(*session->getID());
		authenticate = authenticate && session->isUntrusted();
	}

	if (authenticate)
		req.options.userPassword.setCredentials(*proxifier->getUsername(), *proxifier->getPassword());

	if (sessionSupplicant)
		sessionSupplicant->process(&req);

	S6U::RequestSafety::Recommendation recommendation = S6U::RequestSafety::recommend(req, dstSock.tls != nullptr, buf.usedSize());
	if (recommendation.useToken && session)
	{
		optional<uint32_t> token = session->getToken();
		if (token)
		{
			req.options.idempotence.setToken(token.get());
			recommendation.tokenSpent(dstSock.tls != nullptr);
		}
	}

	uint8_t reqBuf[HEADROOM];
	S6M::ByteBuffer bb(reqBuf, sizeof(reqBuf));
	req.pack(&bb);
	buf.prepend(bb.getBuf(), bb.getUsed());

	/* connect */
	dstSock.sockConnect(*proxifier->getProxyAddr(), &buf, recommendation.tfoPayload, recommendation.earlyData);

	poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
}

void ProxifierUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_CONNECTING:
	{
		int err;
		socklen_t errLen = sizeof(err);

		int rc = getsockopt(dstSock.fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
		if (rc < 0)
			throw system_error(errno, system_category());
		if (err != 0)
			throw system_error(err, system_category());
		
		state = S_HANDSHAKING;
		[[fallthrough]];
	}
	case S_HANDSHAKING:
	{
		dstSock.clientHandshake(&buf);
		state = S_SENDING_REQ;
		[[fallthrough]];
	}
	case S_SENDING_REQ:
	{
		if (buf.usedSize() > 0)
		{
			ssize_t bytes = dstSock.sockSend(&buf);
			if (bytes == 0)
				return;
		}
		
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
