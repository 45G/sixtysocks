#include "poller.hh"
#include "stickreactor.hh"

StickReactor::~StickReactor()
{
	try
	{
		poller->remove(sock);
	}
	catch(...) {}
}

void StickReactor::deactivate()
{
	Reactor::deactivate();
	poller->remove(sock);
}
