#ifndef RESOLVER_HH
#define RESOLVER_HH

#include "proxyupstreamer.hh"

class Resolver: public Reactor
{
public:
	using Reactor::Reactor;
	
	virtual void start();
	
	virtual void process(int fd, uint32_t events);
	
	void resolve(boost::intrusive_ptr<ProxyUpstreamer> upstreamer, const std::string &hostname);
};

#endif // RESOLVER_HH
