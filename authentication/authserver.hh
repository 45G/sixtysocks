#ifndef NOAUTHSERVER_HH
#define NOAUTHSERVER_HH

#include "../core/streamreactor.hh"

class ProxyUpstreamer;

class AuthServer: public Reactor
{
	enum State
	{
		S_WRITING,
		S_DONE,
	};
	
	boost::intrusive_ptr<ProxyUpstreamer> upstreamer;

	State state;

	StreamBuffer buf;

public:
	AuthServer(ProxyUpstreamer *upstreamer);

	void process(int fd, uint32_t events);

	void start(bool defer = false);
	
	void deactivate();
	
	void mayRead();
};

#endif // NOAUTHSERVER_HH
