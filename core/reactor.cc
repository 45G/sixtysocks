#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <system_error>
#include "poller.hh"
#include "reactor.hh"

using namespace std;

void Reactor::deactivate()
{
	tbb::spin_mutex::scoped_lock scopedLock(deactivationLock);
	active = false;
}

Reactor::~Reactor() {}
