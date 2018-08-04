#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <system_error>
#include "poller.hh"
#include "reactor.hh"

using namespace std;

void Reactor::deactivate()
{
	active = false;
}

Reactor::~Reactor() {}

size_t Reactor::tcpRecv(int fd, StreamBuffer *buf)
{
	ssize_t bytes = recv(fd, buf->getTail(), buf->availSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw RescheduleException(this, fd, Poller::IN_EVENTS);
		throw std::system_error(errno, std::system_category());
	}
	buf->use(bytes);
	return bytes;
}

size_t Reactor::tcpSend(int fd, StreamBuffer *buf)
{
	ssize_t bytes = send(fd, buf->getHead(), buf->usedSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw RescheduleException(this, fd, Poller::OUT_EVENTS);
		throw system_error(errno, system_category());
	}
	buf->unuseHead(bytes);
	return bytes;
}

size_t Reactor::tcpSendTFO(int fd, StreamBuffer *buf, S6U::SocketAddress dest)
{
	ssize_t bytes = sendto(fd, buf->getHead(), buf->usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
	if (bytes < 0 && errno != EINPROGRESS)
		throw system_error(errno, system_category());
	if (bytes > 0)
		buf->unuseHead(bytes);
	return bytes;
}

void Reactor::tcpConnect(int fd, S6U::SocketAddress dest)
{
	int rc = connect(fd, &dest.sockAddress, dest.size());
	if (rc < 0 && errno != EINPROGRESS)
		throw system_error(errno, system_category());
}
