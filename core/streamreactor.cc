#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "../core/sockio.hh"
#include "streamreactor.hh"

using namespace std;

int StreamReactor::fill(int fd)
{
	ssize_t bytes = recv(fd, buf.getTail(), buf.availSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw ReschedDisposition(fd, Poller::IN_EVENTS);
		throw system_error(errno, system_category());
	}
	buf.use(bytes);
	return bytes;
}

int StreamReactor::spill(int fd)
{
	ssize_t bytes = send(fd, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
			throw ReschedDisposition(fd, Poller::OUT_EVENTS);
		throw system_error(errno, system_category());
	}
	buf.unuseHead(bytes);
	return bytes;
}

int StreamReactor::spillTFO(int fd, S6U::SocketAddress dest)
{
	ssize_t bytes = sendto(fd, buf.getHead(), buf.usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
	if (bytes > 0)
		buf.unuseHead(bytes);
	return bytes;
}

void StreamReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	switch (streamState)
	{
	case SS_RECEIVING:
	{
		ssize_t bytes = fill(srcFD);
		if (bytes == 0)
		{
			poller->remove(srcFD);
			srcFD.reset();
			if (buf.usedSize() == 0)
			{
				poller->remove(dstFD);
				dstFD.reset();
				return;
			}
		}

		streamState = SS_SENDING;
		[[fallthrough]];
	}
	case SS_SENDING:
	{
		ssize_t bytes = spill(dstFD);
		if (bytes == 0)
		{
			poller->remove(srcFD);
			srcFD.reset();
			poller->remove(dstFD);
			dstFD.reset();
			return;
		}

		if (buf.usedSize() == 0)
		{
			streamState = SS_RECEIVING;
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}
		else
		{
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		}

		break;
	}
	}
}

void StreamReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(srcFD);
	poller->remove(dstFD);
}

void StreamReactor::start()
{
	switch (streamState)
	{
	case SS_RECEIVING:
		poller->add(this, srcFD, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, dstFD, Poller::OUT_EVENTS);
		break;
	}
}

StreamReactor::~StreamReactor()
{
	try
	{
		poller->remove(srcFD);
		poller->remove(dstFD);
	}
	catch(...) {}
}
