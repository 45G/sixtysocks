#include "authenticator.hh"

using namespace std;

void Authenticator::deactivate()
{
	Reactor::deactivate();
	owner->deactivate();
}
