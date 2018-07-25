#ifndef WINDOWSUPPLICANT_HH
#define WINDOWSUPPLICANT_HH

#include <socks6msg/socks6msg.hh>

class Proxifier;

class WindowSupplicant
{
	Proxifier *proxifier;
	bool done;
public:
	WindowSupplicant(Proxifier *proxifier)
		: proxifier(proxifier), done(false) {}
	
	~WindowSupplicant();
	
	void process(S6M::Request *req);
	
	void process(S6M::AuthenticationReply *authRep);
};

#endif // WINDOWSUPPLICANT_HH
