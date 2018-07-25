#include "proxifier.hh"
#include "windowsupplicant.hh"

WindowSupplicant::~WindowSupplicant()
{
	if (!done)
		proxifier->supplicantDone();
}

void WindowSupplicant::process(S6M::Request *req)
{
	req->getOptionSet()->requestTokenWindow(200); //TODO: don't hardcode
}

void WindowSupplicant::process(S6M::AuthenticationReply *authRep)
{
	uint32_t size = authRep->getOptionSet()->getTokenWindowSize();
	if (size > 0)
	{
		uint32_t base = authRep->getOptionSet()->getTokenWindowBase();
		
		proxifier->setWallet(boost::shared_ptr<LockableTokenWallet>(new LockableTokenWallet(base, size)));
	}
	proxifier->supplicantDone();
	done = true;
}
