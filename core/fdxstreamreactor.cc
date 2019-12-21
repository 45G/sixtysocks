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

void FDXStreamReactor::upStreamProcess(int fd, uint32_t events)
{
	streamProcess(&upStream, fd, events);
}

void FDXStreamReactor::downStreamProcess(int fd, uint32_t events)
{
	streamProcess(&downStream, fd, events);
}

void FDXStreamReactor::process(int fd, uint32_t events)
{
	if (fd == upStream.srcSock.fd || fd == upStream.dstSock.fd)
		upStreamProcess(fd, events);
	else if (fd == downStream.srcSock.fd || fd == downStream.dstSock.fd)
		downStreamProcess(fd, events);
	else
		assert(false);
}

void FDXStreamReactor::deactivate()
{
	Reactor::deactivate();
	
	for (Stream *stream: { &upStream, &downStream })
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
	if (upStream.buf.usedSize() > 0)
		upStream.state = SS_SENDING;
	else
		upStream.state = SS_RECEIVING;
	
	switch (upStream.state)
	{
	case SS_RECEIVING:
		poller->add(this, upStream.srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, upStream.dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

void FDXStreamReactor::downStreamStart()
{
	if (downStream.buf.usedSize() > 0)
		downStream.state = SS_SENDING;
	else
		downStream.state = SS_RECEIVING;
	
	switch (downStream.state)
	{
	case SS_RECEIVING:
		poller->add(this, downStream.srcSock.fd, Poller::IN_EVENTS);
		break;

	case SS_SENDING:
		poller->add(this, downStream.dstSock.fd, Poller::OUT_EVENTS);
		break;
	}
}

FDXStreamReactor::~FDXStreamReactor()
{
	try
	{
		for (Stream *stream: { &upStream, &downStream })
		{
			poller->remove(stream->srcSock.fd);
			poller->remove(stream->dstSock.fd);
		}
	}
	catch(...) {}
}
