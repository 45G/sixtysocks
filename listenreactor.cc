#include <sys/socket.h>
#include <errno.h>
#include <system_error>
#include "listenreactor.hh"

using namespace std;

void ListenReactor::processError()
{
	int err;
	socklen_t errLen;
	
	int rc = getsockopt(listenFD, SOL_SOCKET, SO_ERROR, &err, &errLen);
	if (rc < 0)
		throw system_error(errno, system_category());
	
	processError(err);
}

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
	case EPROTO:
		break;
		
	default:
		throw system_error(err, system_category());
	}
}

int ListenReactor::getFD() const
{
	return listenFD;
}
