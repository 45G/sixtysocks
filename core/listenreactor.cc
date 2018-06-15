#include <sys/socket.h>
#include <errno.h>
#include <system_error>
#include <unistd.h>
#include "poller.hh"
#include "listenreactor.hh"

using namespace std;

void ListenReactor::processError(int err)
{
	switch (err)
	{
	case ECONNABORTED:
	case EINTR:
	case EMFILE:
	case ENFILE:
	case ENOBUFS:
	case ENOMEM:
		break;
		
	default:
		throw system_error(err, system_category());
	}
}

void ListenReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(listenFD);
}

ListenReactor::~ListenReactor()
{
	poller->remove(listenFD);
	close(listenFD);
}
