#include "authutil.hh"

using namespace std;
using namespace S6M;
using namespace S6U;

namespace AuthUtil
{

AuthenticationReply authenticate(Request *req, Proxy *proxy)
{
	AuthenticationReply reply { SOCKS6_AUTH_REPLY_FAILURE };
	shared_ptr<ServerSession> session;

	/* existing session */
	auto rawID = req->options.session.getID();
	if (rawID)
	{
		if (rawID->size() != sizeof(uint64_t))
		{
			reply.options.session.signalReject();
			return reply;
		}

		uint64_t id;
		memcpy(&id, rawID->data(), sizeof(uint64_t));

		session = proxy->getSession(id);
		if (!session)
		{
			reply.options.session.signalReject();
			return reply;
		}
		reply.options.session.signalOK();
	}

	/* authenticate */
	auto checker = proxy->getPasswordChecker();
	if (checker && !session)
	{
		bool success = checker->check(req->options.userPassword.getUsername(), req->options.userPassword.getPassword());
		reply.options.userPassword.setReply(success);
		if (!success)
			return reply;
	}

	/* new session */
	if (!session && req->options.session.requested())
	{
		session = proxy->spawnSession();

		uint64_t id = session->getID();
		SessionID rawID;
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
				return reply;
			}

			bool success = session->getTokenBank()->withdraw(token.value());
			reply.options.idempotence.setReply(success);
			if (!success)
				return reply;
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
	return reply;
}

}
