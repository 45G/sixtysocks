#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "fdxstreamreactor.hh"

using namespace std;

void FDXStreamReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	switch (streamState)
	{
	case SS_RECEIVING:
	{
		ssize_t bytes = srcSock.sockRecv(&buf);
		if (bytes == 0 && buf.usedSize() == 0)
		{
			deactivate();
			return;
		}

		streamState = SS_SENDING;
		[[fallthrough]];
	}
	case SS_SENDING:
	{
		ssize_t bytes = dstSock.sockSend(&buf);
		if (bytes == 0)
		{
			deactivate();
			return;
		}

		if (buf.usedSize() == 0)
		{
			streamState = SS_RECEIVING;
			poller->add(this, srcSock.fd, Poller::IN_EVENTS);
		}
		else
		{
			poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
		}

		break;
	}
	}
}

void FDXStreamReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(srcSock.fd);
	poller->remove(dstSock.fd);
}

void FDXStreamReactor::start()
{
	if (buf.usedSize() > 0)
		streamState = SS_SENDING;
	else
		streamState = SS_RECEIVING;
	
	switch (streamState)
	{
	case SS_RECEIVING:
		poller->add(this, srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

FDXStreamReactor::~FDXStreamReactor()
{
	try
	{
		poller->remove(srcSock.fd);
		poller->remove(dstSock.fd);
	}
	catch(...) {}
}
