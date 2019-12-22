#include <unistd.h>
#include <system_error>
#include "../core/poller.hh"
#include "fdxstreamreactor.hh"

using namespace std;

void FDXStreamReactor::streamProcess(Stream *stream, int fd, uint32_t events)
{
	(void)fd; (void)events;

	switch (stream ->state)
	{
	case SS_RECEIVING:
	{
		ssize_t bytes = stream ->srcSock.sockRecv(&stream ->buf);
		if (bytes == 0 && stream ->buf.usedSize() == 0)
		{
			deactivate();
			return;
		}

		stream ->state = SS_SENDING;
		[[fallthrough]];
	}
	case SS_SENDING:
	{
		ssize_t bytes = stream ->dstSock.sockSend(&stream ->buf);
		if (bytes == 0)
		{
			deactivate();
			return;
		}

		if (stream ->buf.usedSize() == 0)
		{
			stream ->state = SS_RECEIVING;
			poller->add(this, stream ->srcSock.fd, Poller::IN_EVENTS);
		}
		else
		{
			poller->add(this, stream ->dstSock.fd, Poller::OUT_EVENTS);
		}

		break;
	}
	}
}

void FDXStreamReactor::upstreamProcess(int fd, uint32_t events)
{
	streamProcess(&upstream, fd, events);
}

void FDXStreamReactor::downstreamProcess(int fd, uint32_t events)
{
	streamProcess(&downstream, fd, events);
}

void FDXStreamReactor::process(int fd, uint32_t events)
{
	if (fd == upstream.srcSock.fd || fd == upstream.dstSock.fd)
		upstreamProcess(fd, events);
	else if (fd == downstream.srcSock.fd || fd == downstream.dstSock.fd)
		downstreamProcess(fd, events);
	else
		assert(false);
}

void FDXStreamReactor::deactivate()
{
	Reactor::deactivate();
	
	for (Stream *stream: { &upstream, &downstream })
	{
		poller->remove(stream->srcSock.fd);
		poller->remove(stream->dstSock.fd);
	}
}

void FDXStreamReactor::start()
{
	upStreamStart();
}

void FDXStreamReactor::upStreamStart()
{
	if (upstream.buf.usedSize() > 0)
		upstream.state = SS_SENDING;
	else
		upstream.state = SS_RECEIVING;
	
	switch (upstream.state)
	{
	case SS_RECEIVING:
		poller->add(this, upstream.srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, upstream.dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

void FDXStreamReactor::downStreamStart()
{
	if (downstream.buf.usedSize() > 0)
		downstream.state = SS_SENDING;
	else
		downstream.state = SS_RECEIVING;
	
	switch (downstream.state)
	{
	case SS_RECEIVING:
		poller->add(this, downstream.srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, downstream.dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

FDXStreamReactor::~FDXStreamReactor()
{
	try
	{
		for (Stream *stream: { &upstream, &downstream })
		{
			poller->remove(stream->srcSock.fd);
			poller->remove(stream->dstSock.fd);
		}
	}
	catch(...) {}
}
