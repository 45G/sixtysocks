#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "sessionsupplicationagent.hh"

using namespace std;

SessionSupplicationAgent::SessionSupplicationAgent(Proxifier *proxifier, std::shared_ptr<SessionSupplicant> supplicant, TLSContext *clientCtx)
	: StickReactor(proxifier->getPoller()), proxifier(proxifier), supplicant(supplicant), clientCtx(clientCtx)
{
	const S6U::SocketAddress *proxyAddr = proxifier->getProxyAddr();

	sock.fd.assign(socket(proxyAddr->sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (sock.fd < 0)
		throw system_error(errno, system_category());
	if (clientCtx != nullptr)
		sock.tls = new TLS(clientCtx, sock.fd);
	
	S6M::Request req(SOCKS6_REQUEST_NOOP, S6U::Socket::QUAD_ZERO, 0);
	if (proxifier->getUsername()->length() > 0)
		req.options.userPassword.setCredentials(*proxifier->getUsername(), *proxifier->getPassword());
	supplicant->process(&req);

	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
	req.pack(&bb);
	buf.use(bb.getUsed());
}

void SessionSupplicationAgent::start()
{
	sock.sockConnect(*proxifier->getProxyAddr(), &buf, 0, false);

	poller->add(this, sock.fd, Poller::OUT_EVENTS);
}

void SessionSupplicationAgent::process(int fd, uint32_t events)
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
