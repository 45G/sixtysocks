#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "fdxstreamreactor.hh"

using namespace std;

void FDXStreamReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	switch (stream.state)
	{
	case SS_RECEIVING:
	{
		ssize_t bytes = stream.srcSock.sockRecv(&stream.buf);
		if (bytes == 0 && stream.buf.usedSize() == 0)
		{
			deactivate();
			return;
		}

		stream.state = SS_SENDING;
		[[fallthrough]];
	}
	case SS_SENDING:
	{
		ssize_t bytes = stream.dstSock.sockSend(&stream.buf);
		if (bytes == 0)
		{
			deactivate();
			return;
		}

		if (stream.buf.usedSize() == 0)
		{
			stream.state = SS_RECEIVING;
			poller->add(this, stream.srcSock.fd, Poller::IN_EVENTS);
		}
		else
		{
			poller->add(this, stream.dstSock.fd, Poller::OUT_EVENTS);
		}

		break;
	}
	}
}

void FDXStreamReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(stream.srcSock.fd);
	poller->remove(stream.dstSock.fd);
}

void FDXStreamReactor::start()
{
	if (stream.buf.usedSize() > 0)
		stream.state = SS_SENDING;
	else
		stream.state = SS_RECEIVING;
	
	switch (stream.state)
	{
	case SS_RECEIVING:
		poller->add(this, stream.srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, stream.dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

FDXStreamReactor::~FDXStreamReactor()
{
	try
	{
		poller->remove(stream.srcSock.fd);
		poller->remove(stream.dstSock.fd);
	}
	catch(...) {}
}
