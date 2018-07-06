#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "../core/streamreactor.hh"
#include "../proxy/proxyupstreamer.hh"
#include "noauthserver.hh"

using namespace std;

NoAuthServer::NoAuthServer(ProxyUpstreamer *owner)
	: Authenticator(owner, true), state(S_WRITING)
{
	S6M::AuthenticationReply rep(SOCKS6_AUTH_REPLY_SUCCESS, SOCKS6_METHOD_NOAUTH);
	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());

	rep.pack(&bb);
}

void NoAuthServer::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	int bytes = send(owner->getSrcFD(), buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
	if (bytes == 0)
		deactivate();
	if (bytes < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			throw system_error(errno, system_category());
		bytes = 0;
	}
	buf.unuse(bytes);

	if (buf.usedSize() > 0)
		poller->add(this, owner->getSrcFD(), Poller::OUT_EVENTS);
	else
		owner->resume();
}

void NoAuthServer::resume()
{
	poller->add(this, owner->getSrcFD(), Poller::OUT_EVENTS);
}
