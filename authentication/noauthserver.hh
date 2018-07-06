#ifndef NOAUTHSERVER_HH
#define NOAUTHSERVER_HH

#include "../core/authenticator.hh"

class ProxyUpstreamer;

class NoAuthServer: public Authenticator
{
	enum State
	{
		S_WRITING,
		S_DONE,
	};

	State state;

	StreamBuffer buf;

public:
	NoAuthServer(ProxyUpstreamer *owner);

	void process(int fd, uint32_t events);

	void resume();
};

#endif // NOAUTHSERVER_HH
