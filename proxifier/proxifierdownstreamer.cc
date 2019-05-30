#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "../core/poller.hh"
#include "proxifierupstreamer.hh"
#include "proxifierdownstreamer.hh"

using namespace std;

ProxifierDownstreamer::ProxifierDownstreamer(ProxifierUpstreamer *upstreamer)
	: StreamReactor(upstreamer->getPoller()), proxifier(upstreamer->getProxifier()), upstreamer(upstreamer), supplicant(upstreamer->getSupplicant())
{
	srcSock.duplicate(upstreamer->getDstSock());
	dstSock.duplicate(upstreamer->getSrcSock());
}

void ProxifierDownstreamer::process(int fd, uint32_t events)
{
	bool fellThrough = false;

	switch (state)
	{
	case S_WAITING_FOR_AUTH_REP:
	{
		ssize_t bytes = srcSock.sockRecv(&buf);
		if (bytes == 0)
		{
			deactivate();
			return;
		}
		
		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			S6M::AuthenticationReply authRep(&bb);
			if (authRep.getReplyCode() != SOCKS6_AUTH_REPLY_SUCCESS)
			{
				deactivate();
				return;
			}
			buf.unuse(bb.getUsed());
			
			SOCKS6TokenExpenditureCode expenditureCode = authRep.options.idempotence.getReply().get_value_or((SOCKS6TokenExpenditureCode)0);
			if (upstreamer->getWallet().get() != nullptr && (expenditureCode == (SOCKS6TokenExpenditureCode)0))
				proxifier->killWallet(upstreamer->getWallet());
			
			if (supplicant.get() != nullptr)
			{
				supplicant->process(&authRep);
			}
			else
			{
				std::shared_ptr<SyncedTokenWallet> wallet = upstreamer->getWallet();
				if (wallet.get() != nullptr)
					wallet->updateWindow(&authRep.options);
			}
		}
		catch (S6M::EndOfBufferException &)
		{
			poller->add(this, srcSock.fd, Poller::IN_EVENTS);
			return;
		}

		state = S_WAITING_FOR_OP_REP;
		fellThrough = true;
		[[fallthrough]];

	}
	case S_WAITING_FOR_OP_REP:
	{
		if (!fellThrough)
		{
			ssize_t bytes = srcSock.sockRecv(&buf);
			if (bytes == 0)
				return;
		}
		
		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			S6M::OperationReply opRep(&bb);
			if (opRep.getCode() != SOCKS6_OPERATION_REPLY_SUCCESS)
				return;
			buf.unuse(bb.getUsed());
		}
		catch (S6M::EndOfBufferException &)
		{
			poller->add(this, srcSock.fd, Poller::IN_EVENTS);
			return;
		}
		
		upstreamer = nullptr;
		
		state = S_STREAM;
		if (buf.usedSize() > 0)
			streamState = SS_SENDING;
		else
			streamState = SS_RECEIVING;
		[[fallthrough]];
	}
	case S_STREAM:
		StreamReactor::process(fd, events);
		break;
	}
}

void ProxifierDownstreamer::deactivate()
{
	StreamReactor::deactivate();
	upstreamer->deactivate();
}
