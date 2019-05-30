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
	{
		uint32_t base = 0;
		uint32_t size = authRep->options.idempotence.advertisedSize();
		if (size > 0)
			uint32_t base = authRep->options.idempotence.advertisedBase().get();
		
		auto session = make_shared<ClientSession>(id, base, size);
		proxifier->setSession(session);
	}
	
	proxifier->supplicantDone();
	done = true;
}
