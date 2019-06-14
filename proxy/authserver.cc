#include <system_error>
#include <algorithm>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "../core/streamreactor.hh"
#include "../proxy/proxyupstreamer.hh"
#include "../proxy/proxy.hh"
#include "authserver.hh"

using namespace std;

void AuthServer::check()
{
	shared_ptr<S6M::Request> req = upstreamer->getRequest();
	shared_ptr<ServerSession> session;

	/* existing session */
	auto rawID = req->options.session.getID();
	if (rawID != nullptr)
	{
		if (rawID->size() != sizeof(uint64_t))
		{
			reply.options.session.signalReject();
			return;
		}

		uint64_t id;
		memcpy(&id, rawID->data(), sizeof(uint64_t));

		session = upstreamer->getProxy()->getSession(id);
		if (session.get() == nullptr)
		{
			reply.options.session.signalReject();
			return;
		}
	}

	/* authenticate */
	auto checker = upstreamer->getProxy()->getPasswordChecker();
	if (checker != nullptr && session.get() == nullptr)
	{
		bool success = checker->check(req->options.userPassword.getUsername(), req->options.userPassword.getPassword());
		reply.options.userPassword.setReply(success);
		if (!success)
			return;
	}

	/* new session */
	if (session.get() == nullptr && req->options.session.requested())
	{
		session = upstreamer->getProxy()->spawnSession();

		uint64_t id = session->getID();
		vector<uint8_t> rawID;
		rawID.resize(sizeof(uint64_t));
		memcpy(rawID.data(), &id, sizeof(uint64_t));

		reply.options.session.setID(rawID);
	}

	/* idempotence stuff */
	if (session.get() != nullptr)
	{
		/* spend token */
		auto token = req->options.idempotence.getToken();
		if (token)
		{
			/* got bank? */
			if (session->getTokenBank() == nullptr)
			{
				reply.options.idempotence.setReply(false);
				return;
			}

			bool success = session->getTokenBank()->withdraw(token.get());
			reply.options.idempotence.setReply(success);
			if (!success)
				return;
		}

		/* new bank */
		session->makeBank(req->options.idempotence.requestedSize());

		/* advert */
		SyncedTokenBank *bank = session->getTokenBank();
		if (bank != nullptr)
		{
			auto window = bank->getWindow();
			reply.options.idempotence.advertise(window.first, window.second);
		}
	}

	reply.setCode(SOCKS6_AUTH_REPLY_SUCCESS);
}

AuthServer::AuthServer(ProxyUpstreamer *upstreamer)
	: StickReactor(upstreamer->getPoller()), upstreamer(upstreamer)
{
	sock.duplicate(upstreamer->getSrcSock());
	
	check();
	
	buf.use(reply.pack(buf.getTail(), buf.availSize()));
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
	else if (reply.getCode() == SOCKS6_AUTH_REPLY_SUCCESS)
	{
		try
		{
			upstreamer->authDone();
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
	process(-1, 0);
}

void AuthServer::deactivate()
{
	StickReactor::deactivate();
	upstreamer->deactivate();
}
