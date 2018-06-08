#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "streamreactor.hh"

using namespace std;

void StreamReactor::process(Poller *poller, uint32_t events)
{
	(void)events;

	switch (streamState)
	{
	case SS_WAITING_TO_RECV:
	{
		ssize_t bytes = fill(srcFD, MSG_NOSIGNAL);
		if (bytes == 0)
		{
			close(srcFD); // tolerable error
			srcFD = -1;
		}
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				close(srcFD); // tolerable error
				srcFD = -1;
			}
		}

		bytes = spill(dstFD, MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}

		if (buf.usedSize() > 0)
		{
			streamState = SS_WAITING_TO_SEND;
			poller->add(this, dstFD, EPOLLOUT);
		}
		else
		{
			poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
		}

		break;
	}
	case SS_WAITING_TO_SEND:
	{
		ssize_t bytes = spill(dstFD, MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}

		if (buf.usedSize() == 0)
		{
			streamState = SS_WAITING_TO_RECV;
			poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
		}
		else
		{
			poller->add(this, dstFD, EPOLLOUT);
		}

		break;
	}
	}
}

int StreamReactor::getFD() const
{
	switch (streamState)
	{
	case StreamReactor::SS_WAITING_TO_RECV:
		return srcFD;

	case StreamReactor::SS_WAITING_TO_SEND:
		return dstFD;
	}

	return -1;
}

StreamReactor::~StreamReactor()
{
	if (srcFD != -1)
	{
		shutdown(srcFD, SHUT_RD);
		close(srcFD);
	}
	if (dstFD != -1)
	{
		shutdown(dstFD, SHUT_WR);
		close(dstFD);
	}
}
