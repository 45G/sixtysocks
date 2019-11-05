#include "proxifier.hh"
#include "sessionsupplicant.hh"

using namespace std;
using namespace boost;

SessionSupplicant::~SessionSupplicant()
{
	if (!done)
		proxifier->supplicantDone();
}

void SessionSupplicant::process(S6M::Request *req)
{
	req->options.session.request();
	req->options.idempotence.request(200); //TODO: don't hardcode
}

void SessionSupplicant::process(S6M::AuthenticationReply *authRep)
{
	auto id = authRep->options.session.getID();
	if (id)
		proxifier->setSession(make_shared<ClientSession>(*id, authRep->options.session.isUntrusted(), authRep->options.idempotence.getAdvertised()));
	
	proxifier->supplicantDone();
	done = true;
}
