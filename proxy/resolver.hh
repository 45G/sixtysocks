#ifndef RESOLVER_HH
#define RESOLVER_HH

#include "proxyupstreamer.hh"

class Resolver: public Reactor
{
	std::atomic<uint16_t> tid { 0 };
	
public:
	using Reactor::Reactor;
	
	virtual void start();
	
	virtual void process(int fd, uint32_t events);
	
	void resolve(boost::intrusive_ptr<ProxyUpstreamer> upstreamer, const std::string &hostname);
};

#endif // RESOLVER_HH
