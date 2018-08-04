#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "streamreactor.hh"

using namespace std;

void StreamReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	switch (streamState)
	{
	case SS_RECEIVING:
	{
		ssize_t bytes = tcpRecv(&srcFD, &buf);
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
		ssize_t bytes = tcpSend(&dstFD, &buf);
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
