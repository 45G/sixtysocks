#include "authutil.hh"

using namespace std;
using namespace S6M;
using namespace S6U;

namespace AuthUtil
{

AuthenticationReply authenticate(OptionSet *opts, Proxy *proxy)
{
	AuthenticationReply reply { SOCKS6_AUTH_REPLY_FAILURE };
	shared_ptr<ServerSession> session;

	/* existing session */
	auto rawID = opts->session.getID();
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
		bool success = checker->check(opts->userPassword.getCredentials());
		reply.options.userPassword.setReply(success);
		if (!success)
			return reply;
	}

	/* new session */
	if (!session && opts->session.requested())
	{
		session = proxy->spawnSession();

		uint64_t id = session->getID();
		SessionID rawID;
		rawID.resize(sizeof(uint64_t));
		memcpy(rawID.data(), &id, sizeof(uint64_t));

		reply.options.session.setID(rawID);
	}

	/* idempotence stuff; it depends on having a valid session */
	if (!session)
	{
		reply.code = SOCKS6_AUTH_REPLY_SUCCESS;
		return reply;
	}
	
	/* new bank */
	session->makeBank(opts->idempotence.requestedSize());
	
	SyncedTokenBank *bank = session->getTokenBank();
	
	/* advert */
	if (bank)
	{
		auto window = bank->getWindow();
		reply.options.idempotence.advertise(window);
	}
	
	/* spend token */
	auto token = opts->idempotence.getToken();
	if (!token)
	{
		reply.code = SOCKS6_AUTH_REPLY_SUCCESS;
		return reply;
	}
	/* got bank? */
	if (!bank || !bank->withdraw(token.value()))
	{
		reply.options.idempotence.setReply(false);
		return reply;
	}
	reply.options.idempotence.setReply(true);
	reply.code = SOCKS6_AUTH_REPLY_SUCCESS;
	return reply;
}

}
