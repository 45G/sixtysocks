#include "proxifier.hh"
#include "windowsupplicant.hh"

WindowSupplicant::~WindowSupplicant()
{
	if (!done)
		proxifier->supplicantDone();
}

void WindowSupplicant::process(S6M::Request *req)
{
	req->getOptionSet()->idempotence.request(200); //TODO: don't hardcode
}

void WindowSupplicant::process(S6M::AuthenticationReply *authRep)
{
	uint32_t size = authRep->getOptionSet()->idempotence.advertisedSize();
	if (size > 0)
	{
		uint32_t base = authRep->getOptionSet()->idempotence.advertisedBase().get();
		
		proxifier->setWallet(std::shared_ptr<SyncedTokenWallet>(new SyncedTokenWallet(base, size)));
	}
	proxifier->supplicantDone();
	done = true;
}
