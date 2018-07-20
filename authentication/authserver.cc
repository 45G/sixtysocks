#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "../core/streamreactor.hh"
#include "../proxy/proxyupstreamer.hh"
#include "../proxy/proxy.hh"
#include "authserver.hh"

using namespace std;

AuthServer::AuthServer(ProxyUpstreamer *upstreamer)
	: Reactor(upstreamer->getPoller()), upstreamer(upstreamer), state(S_WRITING)
{
	SOCKS6AuthReplyCode code;
	SOCKS6Method method;
	
	PasswordChecker *checker = upstreamer->getProxy()->getPasswordChecker();
	if (checker == NULL)
	{
		code = SOCKS6_AUTH_REPLY_SUCCESS;
		method = SOCKS6_METHOD_NOAUTH;
		success = true;
	}
	else if (checker->check(upstreamer->getRequest()->getOptionSet()->getUsername(), upstreamer->getRequest()->getOptionSet()->getPassword()))
	{
		code = SOCKS6_AUTH_REPLY_SUCCESS;
		method = SOCKS6_METHOD_USRPASSWD;
		success = true;
	}
	else
	{
		code = SOCKS6_AUTH_REPLY_MORE;
		method = SOCKS6_METHOD_UNACCEPTABLE;
		success = false;
	}
	
	S6M::AuthenticationReply rep(code, method);
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
	else if (success)
		((ProxyUpstreamer *)upstreamer.get())->authDone((SOCKS6TokenExpenditureCode)0);
	else
		upstreamer->deactivate();
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
