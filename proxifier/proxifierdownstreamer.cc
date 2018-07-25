#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "proxifierupstreamer.hh"
#include "proxifierdownstreamer.hh"

using namespace std;

ProxifierDownstreamer::ProxifierDownstreamer(ProxifierUpstreamer *upstreamer)
	: StreamReactor(upstreamer->getPoller(), -1, -1), proxifier(upstreamer->getProxifier()), upstreamer(upstreamer), state(S_WAITING_FOR_AUTH_REP), supplicant(upstreamer->getSupplicant())
{
	srcFD = dup(upstreamer->getDstFD());
	if (srcFD < 0)
		throw system_error(errno, system_category());
	
	dstFD = dup(upstreamer->getSrcFD());
	if (dstFD < 0)
	{
		close(srcFD); // tolerable error
		throw system_error(errno, system_category());
	}
}

void ProxifierDownstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_WAITING_FOR_AUTH_REP:
	{
		ssize_t bytes = fill(srcFD);
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
			buf.unuseHead(bb.getUsed());
			
			SOCKS6TokenExpenditureCode expenditureCode = authRep.getOptionSet()->getExpenditureReply();
			if (upstreamer->getWallet().get() != NULL && (expenditureCode == (SOCKS6TokenExpenditureCode)0 || expenditureCode == SOCKS6_TOK_EXPEND_NO_WND))
				proxifier->killWallet(upstreamer->getWallet());
			
			if (supplicant.get() != NULL)
			{
				supplicant->process(&authRep);
			}
			else
			{
				boost::shared_ptr<LockableTokenWallet> wallet = upstreamer->getWallet();
				wallet->acquire();
				wallet->updateWindow(authRep.getOptionSet());
				wallet->release();
			}
			
			state = S_WAITING_FOR_OP_REP;
		}
		catch (S6M::EndOfBufferException) {}
		
		poller->add(this, srcFD, Poller::IN_EVENTS);
		
		break;
	}
	case S_WAITING_FOR_OP_REP:
	{
		ssize_t bytes = fill(srcFD);
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
			buf.unuseHead(bb.getUsed());
			state = S_WAITING_FOR_OP_REP;
		}
		catch (S6M::EndOfBufferException)
		{
			poller->add(this, srcFD, Poller::IN_EVENTS);
			return;
		}
		
		upstreamer = NULL;
		
		state = S_STREAM;
		if (buf.usedSize() > 0)
		{
			ssize_t bytes = spill(dstFD);
			if (bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
				return;
		}
		if (buf.usedSize() == 0)
		{
			streamState = SS_WAITING_TO_RECV;
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}
		else
		{
			streamState = SS_WAITING_TO_SEND;
			poller->add(this, srcFD, Poller::OUT_EVENTS);
		}
		
		break;
	}
	case S_STREAM:
		StreamReactor::process(fd, events);
		break;
	}
}
