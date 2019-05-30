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
	bool pwChecked;
	std::shared_ptr<S6M::Request> req = upstreamer->getRequest();
	Proxy *proxy = upstreamer->getProxy();
	
	PasswordChecker *checker = proxy->getPasswordChecker();
	if (checker == nullptr)
	{
		code = SOCKS6_AUTH_REPLY_SUCCESS;
		success = true;
	}
	else if (checker->check(req->options.userPasswd.getUsername(), req->options.userPasswd.getPassword()))
	{
		code = SOCKS6_AUTH_REPLY_SUCCESS;
		pwChecked = true;
		success = true;
	}
	else
	{
		code = SOCKS6_AUTH_REPLY_MORE;
		success = false;
	}
	
	S6M::AuthenticationReply rep(code);
	
	bool idempotenceFail = false;
	
	//TODO: untangle mess
	SyncedTokenBank *bank = nullptr;
	if (success && pwChecked)
		bank = proxy->getBank(*req->options.userPasswd.getUsername());
	
	/* spend token? */
	if (success && (bool)req->options.idempotence.getToken())
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
			uint32_t token = req->options.idempotence.getToken().get();
			
			expendCode = bank->withdraw(token) ? SOCKS6_TOK_EXPEND_SUCCESS : SOCKS6_TOK_EXPEND_FAILURE;
			
			if (expendCode == SOCKS6_TOK_EXPEND_FAILURE)
			{
				idempotenceFail = true;
				upstreamer->fail();
			}
		}
		rep.options.idempotence.setReply(expendCode);
	}
	
	/* request window */
	uint32_t requestedWindow = req->options.idempotence.requestedSize();
	if (success && pwChecked && !idempotenceFail && requestedWindow > 0)
	{
		if (bank == nullptr)
			bank = proxy->createBank(*req->options.userPasswd.getUsername(), std::min(requestedWindow, (uint32_t)200)); //TODO: don't hardcode
		else
			bank->renew();
	}
	
	/* advertise window */
	if (bank != nullptr)
	{
		uint32_t base;
		uint32_t size;
		
		bank->getWindow(&base, &size);
		rep.options.idempotence.advertise(base, size);
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
