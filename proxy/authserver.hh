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
	
	bool success;
	
	void sendReply();

public:
	AuthServer(ProxyUpstreamer *upstreamer);

	void process(int fd, uint32_t events);

	void start();
	
	void deactivate();
	
	//TODO: in case long auth is implemented
	//void mayRead();
};

#endif // AUTHSERVER_HH
