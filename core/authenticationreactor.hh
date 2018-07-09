#ifndef AUTHENTICATIONREACTOR_HH
#define AUTHENTICATIONREACTOR_HH

#include <boost/intrusive_ptr.hpp>
#include "streamreactor.hh"

class AuthenticationReactor: public Reactor
{
protected:
	boost::intrusive_ptr<StreamReactor> owner;

public:
	AuthenticationReactor(StreamReactor *owner)
		: Reactor(owner->getPoller()), owner(owner) {}

public:
	void deactivate();
};

#endif // AUTHENTICATIONREACTOR_HH
