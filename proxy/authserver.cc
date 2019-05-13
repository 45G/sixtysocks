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
	: StickReactor(upstreamer->getPoller()), upstreamer(upstreamer)
{
	sock.duplicate(upstreamer->getSrcSock());
	
	SOCKS6AuthReplyCode code;
	SOCKS6Method method;
	std::shared_ptr<S6M::Request> req = upstreamer->getRequest();
	Proxy *proxy = upstreamer->getProxy();
	
	PasswordChecker *checker = proxy->getPasswordChecker();
	if (checker == nullptr)
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
	SyncedTokenBank *bank = nullptr;
	if (success && method != SOCKS6_METHOD_NOAUTH)
		bank = proxy->getBank(*req->getOptionSet()->getUsername());
	
	/* spend token? */
	if (success && (bool)req->getOptionSet()->idempotence.getToken())
	{	
		SOCKS6TokenExpenditureCode expendCode;
		/* no bank */
		if (bank == nullptr)
		{
			idempotenceFail = false;
			upstreamer->fail();
			expendCode = SOCKS6_TOK_EXPEND_FAILURE;
		}
		else
		{
			uint32_t token = req->getOptionSet()->idempotence.getToken().get();
			
			expendCode = bank->withdraw(token);
			
			if (expendCode == SOCKS6_TOK_EXPEND_FAILURE)
			{
				idempotenceFail = true;
				upstreamer->fail();
			}
		}
		rep.getOptionSet()->idempotence.setReply(expendCode);
	}
	
	/* request window */
	uint32_t requestedWindow = req->getOptionSet()->idempotence.requestedSize();
	if (success && method != SOCKS6_METHOD_NOAUTH && !idempotenceFail && requestedWindow > 0)
	{
		if (bank == nullptr)
			bank = proxy->createBank(*req->getOptionSet()->getUsername(), std::min(requestedWindow, (uint32_t)200)); //TODO: don't hardcode
		else
			bank->renew();
	}
	
	/* advertise window */
	if (bank != nullptr)
	{
		uint32_t base;
		uint32_t size;
		
		bank->getWindow(&base, &size);
		rep.getOptionSet()->idempotence.advertise(base, size);
	}
	
	buf.use(rep.pack(buf.getTail(), buf.availSize()));
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
	else if (success)
	{
		try
		{
			upstreamer->authDone((SOCKS6TokenExpenditureCode)0);
		}
		catch (RescheduleException &resched)
		{
			poller->add(upstreamer, resched.getFD(), resched.getEvents());
		}
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
	sendReply();
}

void AuthServer::deactivate()
{
	StickReactor::deactivate();
	upstreamer->deactivate();
}
