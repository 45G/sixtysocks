#include "authenticationreactor.hh"

using namespace std;

void AuthenticationReactor::deactivate()
{
	Reactor::deactivate();
	upstreamer->deactivate();
}
