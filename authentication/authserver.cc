#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "../core/streamreactor.hh"
#include "../proxy/proxyupstreamer.hh"
#include "authserver.hh"

using namespace std;

AuthServer::AuthServer(ProxyUpstreamer *upstreamer)
	: Reactor(upstreamer->getPoller()), upstreamer(upstreamer), state(S_WRITING)
{
	S6M::AuthenticationReply rep(SOCKS6_AUTH_REPLY_SUCCESS, SOCKS6_METHOD_NOAUTH);
	buf.use(rep.pack(buf.getTail(), buf.availSize()));
}

void AuthServer::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	int bytes = send(upstreamer->getSrcFD(), buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
	if (bytes == 0)
		deactivate();
	if (bytes < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			throw system_error(errno, system_category());
		bytes = 0;
	}
	buf.unuseHead(bytes);

	if (buf.usedSize() > 0)
		poller->add(this, upstreamer->getSrcFD(), Poller::OUT_EVENTS);
	else
		((ProxyUpstreamer *)upstreamer.get())->authDone((SOCKS6TokenExpenditureCode)0);
}

void AuthServer::start(bool defer)
{
	poller->add(this, upstreamer->getSrcFD(), Poller::OUT_EVENTS);
}

void AuthServer::deactivate()
{
	Reactor::deactivate();
	upstreamer->deactivate();
}
