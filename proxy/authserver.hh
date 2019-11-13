#ifndef AUTHSERVER_HH
#define AUTHSERVER_HH

#include "../core/streamreactor.hh"
#include "../core/stickreactor.hh"

class ProxyUpstreamer;

class AuthServer: public StickReactor
{
	enum State
	{
		S_WRITING,
		S_DONE,
	};
	
	boost::intrusive_ptr<ProxyUpstreamer> upstreamer;

	State state = S_WRITING;

	S6M::AuthenticationReply reply { SOCKS6_AUTH_REPLY_FAILURE };
	
	void sendReply();

	void check();

public:
	AuthServer(ProxyUpstreamer *upstreamer);

	void process(int fd, uint32_t events);

	void start();
	
	void deactivate();
};

#endif // AUTHSERVER_HH
