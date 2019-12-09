#ifndef SESSIONSUPPLICANT_HH
#define SESSIONSUPPLICANT_HH

#include <socks6msg/socks6msg.hh>

class Proxifier;

class SessionSupplicant
{
	Proxifier *proxifier;
	bool done = false;
	
public:
	SessionSupplicant(Proxifier *proxifier)
		: proxifier(proxifier) {}
	
	~SessionSupplicant();
	
	void process(S6M::Request *req);
	
	void process(S6M::AuthenticationReply *authRep);
};

#endif // SESSIONSUPPLICANT_HH
