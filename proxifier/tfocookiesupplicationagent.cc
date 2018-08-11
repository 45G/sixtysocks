#include "../core/poller.hh"
#include "proxifier.hh"
#include "tfocookiesupplicationagent.hh"

using namespace std;

TFOCookieSupplicationAgent::TFOCookieSupplicationAgent(Proxifier *proxifier)
	: StickReactor(proxifier->getPoller()), proxifier(proxifier)
{
	const S6U::SocketAddress *proxyAddr = proxifier->getProxyAddr();

	sock.fd.assign(socket(proxyAddr->sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (sock.fd < 0)
		throw system_error(errno, system_category());
}

void TFOCookieSupplicationAgent::start()
{
	sock.sockConnect(*proxifier->getProxyAddr(), &buf, true, true, NULL);
	poller->add(this, sock.fd, Poller::OUT_EVENTS);
}

void TFOCookieSupplicationAgent::process(int fd, uint32_t events)
{
	(void)fd; (void)events;
}
