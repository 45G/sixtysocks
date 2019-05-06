#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <system_error>
#include "poller.hh"
#include "reactor.hh"

using namespace std;

void Reactor::deactivate()
{
	std::lock_guard<Spinlock> scopedLock(deactivationLock);
	active = false;
}

Reactor::~Reactor() {}
