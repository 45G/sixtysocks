#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "windowsupplicationagent.hh"

using namespace std;

WindowSupplicationAgent::WindowSupplicationAgent(Proxifier *proxifier, boost::shared_ptr<WindowSupplicant> supplicant, TLSContext *clientCtx)
	: StickReactor(proxifier->getPoller()), proxifier(proxifier), state(S_CONNECTING), supplicant(supplicant), clientCtx(clientCtx)
{
	const S6U::SocketAddress *proxyAddr = proxifier->getProxyAddr();

	sock.fd.assign(socket(proxyAddr->sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (sock.fd < 0)
		throw system_error(errno, system_category());
	if (clientCtx != NULL)
		sock.tls = new TLS(clientCtx, sock.fd, proxifier->getSession());
	
	S6M::Request req(SOCKS6_REQUEST_NOOP, S6U::Socket::QUAD_ZERO, 0, 0);
	req.getOptionSet()->setUsernamePassword(proxifier->getUsername(), proxifier->getPassword());
	supplicant->process(&req);

	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
	req.pack(&bb);
	buf.use(bb.getUsed());
}

void WindowSupplicationAgent::start()
{
	sock.sockConnect(*proxifier->getProxyAddr(), &buf, false, false);

	poller->add(this, sock.fd, Poller::OUT_EVENTS);
}

void WindowSupplicationAgent::process(int fd, uint32_t events)
{
	(void)fd; (void)events;
	
	switch (state)
	{
	case S_CONNECTING:
	{
		int err;
		socklen_t errLen = sizeof(err);

		int rc = getsockopt(sock.fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
		if (rc < 0)
			throw system_error(errno, system_category());
		if (err != 0)
			throw system_error(err, system_category());
		
		state = S_HANDSHAKING;
		[[fallthrough]];
	}
	case S_HANDSHAKING:
	{
		sock.clientHandshake();
		state = S_SENDING_REQ;
		[[fallthrough]];
	}
	case S_SENDING_REQ:
	{
		ssize_t bytes = sock.sockSend(&buf);
		if (bytes == 0)
			return;
		
		if (buf.usedSize() > 0)
		{
			poller->add(this, sock.fd, Poller::OUT_EVENTS);
		}
		else
		{
			state = S_RECEIVING_AUTH_REP;
			poller->add(this, sock.fd, Poller::IN_EVENTS);
		}
		break;
	}
		
	case S_RECEIVING_AUTH_REP:
	{
		ssize_t bytes = sock.sockRecv(&buf);
		if (bytes == 0)
			return;
		
		try
		{
			S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
			S6M::AuthenticationReply authRep(&bb);
			
			supplicant->process(&authRep);
		}
		catch (S6M::EndOfBufferException &)
		{
			poller->add(this, sock.fd, Poller::IN_EVENTS);
		}
		break;
	}
	}
}
