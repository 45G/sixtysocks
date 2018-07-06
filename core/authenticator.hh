#ifndef AUTHENTICATOR_HH
#define AUTHENTICATOR_HH

#include "reactor.hh"

class Authenticator: public Reactor
{
	int fd;

public:
	Authenticator(Poller *poller, int fd)
		: Reactor(poller), fd(fd) {}
};

#endif // AUTHENTICATOR_HH
