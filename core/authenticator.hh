#ifndef AUTHENTICATOR_HH
#define AUTHENTICATOR_HH

#include <boost/intrusive_ptr.hpp>
#include "streamreactor.hh"

class Authenticator: public Reactor
{
protected:
	boost::intrusive_ptr<StreamReactor> owner;

	bool success;

public:
	Authenticator(StreamReactor *owner, bool success = false)
		: Reactor(owner->getPoller()), owner(owner), success(success) {}

public:
	bool isSuccessful()
	{
		return success;
	}

	void deactivate();
};

#endif // AUTHENTICATOR_HH
