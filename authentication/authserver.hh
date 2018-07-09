#ifndef NOAUTHSERVER_HH
#define NOAUTHSERVER_HH

#include "../core/authenticationreactor.hh"

class ProxyUpstreamer;

class AuthServer: public AuthenticationReactor
{
	enum State
	{
		S_WRITING,
		S_DONE,
	};

	State state;

	StreamBuffer buf;

public:
	AuthServer(ProxyUpstreamer *owner);

	void process(int fd, uint32_t events);

	void start(bool defer = false);
};

#endif // NOAUTHSERVER_HH
