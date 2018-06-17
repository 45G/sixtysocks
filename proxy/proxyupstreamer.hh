#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <boost/intrusive_ptr.hpp>
#include "core/streamreactor.hh"

class Proxy;
class ProxyDownstreamer;

class ProxyUpstreamer: public StreamReactor
{
	bool reqRead;
	
	boost::intrusive_ptr<ProxyDownstreamer> downstreamer;
	
public:
	ProxyUpstreamer(Proxy *owner, int srcFD);
	
	void process(int fd, uint32_t events);
};

#endif // PROXYUPSTREAMER_HH
