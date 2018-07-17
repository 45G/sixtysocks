#ifndef AUTHENTICATIONREACTOR_HH
#define AUTHENTICATIONREACTOR_HH

#include <boost/intrusive_ptr.hpp>
#include "streamreactor.hh"

class AuthenticationReactor: public Reactor
{
protected:
	boost::intrusive_ptr<StreamReactor> upstreamer;

public:
	AuthenticationReactor(StreamReactor *upstreamer)
		: Reactor(upstreamer->getPoller()), upstreamer(upstreamer) {}

public:
	void deactivate();
};

#endif // AUTHENTICATIONREACTOR_HH
