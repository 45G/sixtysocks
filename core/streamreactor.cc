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
		ssize_t bytes = srcSock.sockRecv(&buf);
		if (bytes == 0)
		{
			poller->remove(srcSock.fd);
			srcSock.fd.reset();
			if (buf.usedSize() == 0)
			{
				poller->remove(dstSock.fd);
				dstSock.fd.reset();
				return;
			}
		}

		streamState = SS_SENDING;
		[[fallthrough]];
	}
	case SS_SENDING:
	{
		ssize_t bytes = dstSock.sockSend(&buf);
		if (bytes == 0)
		{
			poller->remove(srcSock.fd);
			srcSock.fd.reset();
			poller->remove(dstSock.fd);
			dstSock.fd.reset();
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

void StreamReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(srcSock.fd);
	poller->remove(dstSock.fd);
}

void StreamReactor::start()
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

StreamReactor::~StreamReactor()
{
	try
	{
		poller->remove(srcSock.fd);
		poller->remove(dstSock.fd);
	}
	catch(...) {}
}
