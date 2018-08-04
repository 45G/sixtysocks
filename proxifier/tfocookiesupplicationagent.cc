#include "../core/poller.hh"
#include "proxifier.hh"
#include "tfocookiesupplicationagent.hh"

using namespace std;

TFOCookieSupplicationAgent::TFOCookieSupplicationAgent(Proxifier *proxifier)
	: Reactor(proxifier->getPoller()), proxifier(proxifier)
{
	const S6U::SocketAddress *proxyAddr = proxifier->getProxyAddr();

	sock.assign(socket(proxyAddr->sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (sock < 0)
		throw system_error(errno, system_category());

	S6M::Request req(SOCKS6_REQUEST_NOOP, S6U::Socket::QUAD_ZERO, 0, 0);
	buf.use(req.pack(buf.getTail(), buf.availSize()));
}

void TFOCookieSupplicationAgent::start()
{
	tcpSendTFO(sock, &buf, *proxifier->getProxyAddr());
	poller->add(this, sock, Poller::OUT_EVENTS);
}

void TFOCookieSupplicationAgent::process(int fd, uint32_t events)
{
	(void)fd; (void)events;
}

void TFOCookieSupplicationAgent::deactivate()
{
	Reactor::deactivate();
	poller->remove(sock);
}
