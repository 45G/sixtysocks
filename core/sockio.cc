#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <system_error>
#include "rescheddisposition.hh"
#include "poller.hh"
#include "sockio.hh"

using namespace std;

size_t sockFill(UniqFD *fd, StreamBuffer *buf)
{
	ssize_t bytes = recv(*fd, buf->getTail(), buf->availSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw ReschedDisposition(*fd, Poller::IN_EVENTS);
		throw std::system_error(errno, std::system_category());
	}
	buf->use(bytes);
	return bytes;
}

size_t sockSpill(UniqFD *fd, StreamBuffer *buf)
{
	ssize_t bytes = send(*fd, buf->getHead(), buf->usedSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw ReschedDisposition(*fd, Poller::OUT_EVENTS);
		throw system_error(errno, system_category());
	}
	buf->unuseHead(bytes);
	return bytes;
}
