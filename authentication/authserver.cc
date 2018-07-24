#include <system_error>
#include <algorithm>
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
	boost::shared_ptr<S6M::Request> req = upstreamer->getRequest();
	Proxy *proxy = upstreamer->getProxy();
	
	PasswordChecker *checker = proxy->getPasswordChecker();
	if (checker == NULL)
	{
		code = SOCKS6_AUTH_REPLY_SUCCESS;
		method = SOCKS6_METHOD_NOAUTH;
		success = true;
	}
	else if (checker->check(req->getOptionSet()->getUsername(), req->getOptionSet()->getPassword()))
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
	
	bool idempotenceFail = false;
	
	//TODO: untangle mess
	LockableTokenBank *bank = NULL;
	if (success && method != SOCKS6_METHOD_NOAUTH)
		bank = proxy->getBank(*req->getOptionSet()->getUsername());
	
	/* spend token? */
	if (success && req->getOptionSet()->hasToken())
	{	
		SOCKS6TokenExpenditureCode expendCode;
		/* no bank */
		if (bank == NULL)
		{
			idempotenceFail = false;
			upstreamer->fail();
			expendCode = SOCKS6_TOK_EXPEND_NO_WND;
		}
		else
		{
			uint32_t token = req->getOptionSet()->getToken();
			
			bank->acquire();
			expendCode = bank->withdraw(token);
			bank->release();
			
			if (expendCode == SOCKS6_TOK_EXPEND_OUT_OF_WND || expendCode == SOCKS6_TOK_EXPEND_DUPLICATE)
			{
				idempotenceFail = true;
				upstreamer->fail();
			}
		}
		rep.getOptionSet()->setExpenditureReply(expendCode);
	}
	
	/* request window */
	uint32_t requestedWindow = req->getOptionSet()->requestedTokenWindow();
	if (success && method != SOCKS6_METHOD_NOAUTH && !idempotenceFail && requestedWindow > 0)
	{
		if (bank == NULL)
		{
			bank = proxy->createBank(*req->getOptionSet()->getUsername(), std::min(requestedWindow, (uint32_t)200)); //TODO: don't hardcode
		}
		else
		{
			bank->acquire();
			bank->renew();
			bank->release();
		}
	}
	
	/* advertise window */
	if (bank != NULL)
	{
		bank->acquire();
		rep.getOptionSet()->setTokenWindow(bank->getBase(), bank->getSize());
		bank->release();
	}
	
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
