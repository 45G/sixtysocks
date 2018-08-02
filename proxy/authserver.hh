#ifndef AUTHSERVER_HH
#define AUTHSERVER_HH

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
	
	bool success;

public:
	AuthServer(ProxyUpstreamer *upstreamer);

	void process(int fd, uint32_t events);

	void start();
	
	void deactivate();
	
	//TODO: in case long auth is implemented
	//void mayRead();
};

#endif // AUTHSERVER_HH
