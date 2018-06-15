#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "streamreactor.hh"

using namespace std;

void StreamReactor::process(Poller *poller)
{
	switch (streamState)
	{
	case SS_WAITING_TO_RECV:
	{
		ssize_t bytes = fill(srcFD);
		if (bytes == 0)
			return;
		if (bytes < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				poller->add(this, srcFD, Poller::IN_EVENTS);
				return;
			}
			throw system_error(errno, system_category());
		}

		bytes = spill(dstFD);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}

		if (buf.usedSize() > 0)
		{
			streamState = SS_WAITING_TO_SEND;
			poller->add(this, dstFD, Poller::OUT_EVENTS);
		}
		else
		{
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}

		break;
	}
	case SS_WAITING_TO_SEND:
	{
		ssize_t bytes = spill(dstFD);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
		}

		if (buf.usedSize() == 0)
		{
			streamState = SS_WAITING_TO_RECV;
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

StreamReactor::~StreamReactor()
{
	if (srcFD != -1)
	{
		try
		{
			poller->remove(srcFD);
		}
		catch(...) {}
		
		shutdown(srcFD, SHUT_RD);  // tolerable error
		close(srcFD); // tolerable error
	}
	if (dstFD != -1)
	{
		try
		{
			poller->remove(dstFD);
		}
		catch(...) {}
		
		shutdown(dstFD, SHUT_WR); // tolerable error
		close(dstFD); // tolerable error
	}
}
