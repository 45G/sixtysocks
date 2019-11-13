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
	if (rawID)
	{
		if (rawID->size() != sizeof(uint64_t))
		{
			reply.options.session.signalReject();
			return;
		}

		uint64_t id;
		memcpy(&id, rawID->data(), sizeof(uint64_t));

		session = upstreamer->getProxy()->getSession(id);
		if (!session)
		{
			reply.options.session.signalReject();
			return;
		}
		reply.options.session.signalOK();
	}

	/* authenticate */
	auto checker = upstreamer->getProxy()->getPasswordChecker();
	if (checker && !session)
	{
		bool success = checker->check(req->options.userPassword.getUsername(), req->options.userPassword.getPassword());
		reply.options.userPassword.setReply(success);
		if (!success)
			return;
	}

	/* new session */
	if (!session && req->options.session.requested())
	{
		session = upstreamer->getProxy()->spawnSession();

		uint64_t id = session->getID();
		vector<uint8_t> rawID;
		rawID.resize(sizeof(uint64_t));
		memcpy(rawID.data(), &id, sizeof(uint64_t));

		reply.options.session.setID(rawID);
	}

	/* idempotence stuff */
	if (session)
	{
		/* spend token */
		auto token = req->options.idempotence.getToken();
		if (token)
		{
			/* got bank? */
			if (!session->getTokenBank())
			{
				reply.options.idempotence.setReply(false);
				return;
			}

			bool success = session->getTokenBank()->withdraw(token.value());
			reply.options.idempotence.setReply(success);
			if (!success)
				return;
		}

		/* new bank */
		session->makeBank(req->options.idempotence.requestedSize());

		/* advert */
		SyncedTokenBank *bank = session->getTokenBank();
		if (bank)
		{
			auto window = bank->getWindow();
			reply.options.idempotence.advertise(window);
		}
	}

	reply.code = SOCKS6_AUTH_REPLY_SUCCESS;
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
	else if (reply.code == SOCKS6_AUTH_REPLY_SUCCESS)
	{
		poller->runAs(upstreamer.get(), [&] {
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
