#include <system_error>
#include <algorithm>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "../core/streamreactor.hh"
#include "../proxy/proxyupstreamer.hh"
#include "../proxy/proxy.hh"
#include "authserver.hh"
#include "authutil.hh"

using namespace std;

AuthServer::AuthServer(ProxyUpstreamer *upstreamer)
	: StickReactor(upstreamer->getPoller()), upstreamer(upstreamer)
{
	sock.duplicate(upstreamer->getSrcSock());
	
	reply = AuthUtil::authenticate(&upstreamer->getRequest()->options, upstreamer->getProxy());
	
	buf.use(reply->pack(buf.getTail(), buf.availSize()));
}

void AuthServer::sendReply()
{
	int bytes = sock.sockSend(&buf);
	if (bytes == 0)
		deactivate();

	if (buf.usedSize() > 0)
	{
		poller->add(this, sock.fd, Poller::OUT_EVENTS);
	}
	else if (reply->code == SOCKS6_AUTH_REPLY_SUCCESS)
	{
		poller->runAs(upstreamer, [&]
		{
			upstreamer->authDone();
		});
	}
	else
	{
		upstreamer->deactivate();
	}
}

void AuthServer::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	sendReply();
}

void AuthServer::start()
{
	process(-1, 0);
}

void AuthServer::deactivate()
{
	StickReactor::deactivate();
	upstreamer->deactivate();
}
