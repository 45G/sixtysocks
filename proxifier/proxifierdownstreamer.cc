#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "proxifierupstreamer.hh"
#include "proxifierdownstreamer.hh"

using namespace std;

ProxifierDownstreamer::ProxifierDownstreamer(ProxifierUpstreamer *upstreamer)
	: StreamReactor(-1, -1), owner(upstreamer->getOwner()), upstreamer(upstreamer), state(S_WAITING_FOR_AUTH_REP)
{
	srcFD = dup(upstreamer->getDstFD());
	if (srcFD < 0)
		throw system_error(errno, system_category());
	
	dstFD = dup(upstreamer->getSrcFD());
	if (dstFD < 0)
	{
		// tolerable error
		close(srcFD);
		throw system_error(errno, system_category());
	}
	upstreamer->use();
}

ProxifierDownstreamer::~ProxifierDownstreamer()
{
	if (upstreamer != NULL)
		upstreamer->unuse();
}

void ProxifierDownstreamer::process(Poller *poller, uint32_t events)
{
	(void)events;

	switch (state)
	{
	case S_WAITING_FOR_AUTH_REP:
	{
		ssize_t bytes = fill(srcFD, MSG_NOSIGNAL);
		if (bytes == 0)
			return;
		if (bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
			return;
		
		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			S6M::AuthenticationReply authRep(&bb);
			if (authRep.getReplyCode() != SOCKS6_AUTH_REPLY_SUCCESS)
				return;
			buf.unuse(bb.getUsed());
			state = S_WAITING_FOR_OP_REP;
		}
		catch (S6M::EndOfBufferException) {}
		
		poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
		
		break;
	}
	case S_WAITING_FOR_OP_REP:
	{
		ssize_t bytes = fill(srcFD, MSG_NOSIGNAL);
		if (bytes == 0)
			return;
		if (bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
			return;
		
		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			S6M::OperationReply opRep(&bb);
			if (opRep.getCode() != SOCKS6_OPERATION_REPLY_SUCCESS)
				return;
			buf.unuse(bb.getUsed());
			state = S_WAITING_FOR_OP_REP;
		}
		catch (S6M::EndOfBufferException)
		{
			poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
			return;
		}
		
		upstreamer->unuse();
		upstreamer = NULL;
		
		state = S_STREAM;
		if (buf.usedSize() > 0)
		{
			ssize_t bytes = spill(dstFD, MSG_NOSIGNAL);
			if (bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
				return;
		}
		if (buf.usedSize() == 0)
		{
			streamState = SS_WAITING_TO_RECV;
			poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
		}
		else
		{
			streamState = SS_WAITING_TO_SEND;
			poller->add(this, srcFD, EPOLLOUT);
		}
		
		
		break;
	}
	case S_STREAM:
		StreamReactor::process(poller, events);
		break;
	}
}

int ProxifierDownstreamer::getFD() const
{
	switch (state)
	{
	case S_WAITING_FOR_AUTH_REP:
	case S_WAITING_FOR_OP_REP:
		return srcFD;

	case S_STREAM:
		return StreamReactor::getFD();
	}
	
	return -1;
}
