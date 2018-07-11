#include <system_error>
#include <socks6util/socks6util.hh>
#include "../core/poller.hh"
#include "proxy.hh"
#include "../authentication/authserver.hh"
#include "connectproxydownstreamer.hh"
#include "simpleproxydownstreamer.hh"
#include "proxyupstreamer.hh"

using namespace std;
using namespace boost;

void ProxyUpstreamer::honorRequest()
{
	if (req->getOptionSet()->expenditureAttempted() && replyOptions.getExpenditureReplyCode() != SOCKS6_TOK_EXPEND_SUCCESS)
	{
		S6M::OperationReply reply(SOCKS6_OPERATION_REPLY_FAILURE, S6M::Address(S6U::Socket::QUAD_ZERO), 0, 0, replyOptions);
		(new SimpleProxyDownstreamer(this, &reply))->start();
		return;
	}
	
	switch (req->getCommandCode())
	{
	case SOCKS6_REQUEST_CONNECT:
	{
		//TODO: resolve
		if (req->getAddress()->getType() == SOCKS6_ADDR_DOMAIN)
		{
			S6M::OperationReply reply(SOCKS6_OPERATION_REPLY_ADDR_NOT_SUPPORTED, S6M::Address(S6U::Socket::QUAD_ZERO), 0, 0, replyOptions);
			(new SimpleProxyDownstreamer(this, &reply))->start();
		}
		
		S6U::SocketAddress addr(*req->getAddress(), req->getPort());
		dstFD = socket(addr.sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
		if (dstFD < 0)
			throw system_error(errno, system_category());
		
		SOCKS6MPTCPScheduler clientProxySched = req->getOptionSet()->getClientProxySched();
		if (S6U::Socket::setMPTCPSched(srcFD, clientProxySched) == 0)
			replyOptions.setClientProxySched(clientProxySched);

		SOCKS6MPTCPScheduler proxyServerSched = req->getOptionSet()->getProxyServerSched();
		if (S6U::Socket::setMPTCPSched(dstFD, proxyServerSched) == 0)
			replyOptions.setProxyServerSched(proxyServerSched);
		
		if (req->getOptionSet()->getTFO())
		{
			ssize_t bytes = spillTFO(dstFD, addr);
			if (bytes < 0 && errno != EINPROGRESS)
					throw system_error(errno, system_category());
		}
		else
		{
			int rc = connect(dstFD, &addr.sockAddress, addr.size());
			if (rc < 0 && errno != EINPROGRESS)
					throw system_error(errno, system_category());
		}
		poller->add(this, dstFD, Poller::OUT_EVENTS);
		state = S_CONNECTING;
		break;
	}
	case SOCKS6_REQUEST_NOOP:
	{
		S6M::OperationReply reply(SOCKS6_OPERATION_REPLY_SUCCESS, S6M::Address(S6U::Socket::QUAD_ZERO), 0, 0, replyOptions);
		(new SimpleProxyDownstreamer(this, &reply))->start();
		break;
	}
	default:
	{
		S6M::OperationReply reply(SOCKS6_OPERATION_REPLY_CMD_NOT_SUPPORTED, S6M::Address(S6U::Socket::QUAD_ZERO), 0, 0, replyOptions);
		(new SimpleProxyDownstreamer(this, &reply))->start();
		break;
	}
	}
}

ProxyUpstreamer::ProxyUpstreamer(Proxy *owner, int srcFD)
	: StreamReactor(owner->getPoller(), srcFD, -1), state(S_READING_REQ), authenticated(false), replyOptions(S6M::OptionSet::M_OP_REP), authServer(NULL) {}

void ProxyUpstreamer::process(int fd, uint32_t events)
{
	switch (state)
	{
	case S_READING_REQ:
	{
		ssize_t bytes = fill(srcFD);
		if (bytes == 0)
			return;
		if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			throw system_error(errno, system_category());

		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			req = boost::shared_ptr<S6M::Request>(new S6M::Request(&bb));
			buf.unuseHead(bb.getUsed());
		}
		catch (S6M::EndOfBufferException)
		{
			poller->add(this, srcFD, Poller::IN_EVENTS);
			return;
		}

		authServer = new AuthServer(this);
		authServer->start();
		
		if (buf.usedSize() < req->getInitialDataLen())
		{
			state = S_READING_INIT_DATA;
			poller->add(this, srcFD, Poller::IN_EVENTS);
		}
		else
		{
			honorLock.acquire();
			state = S_AWAITING_AUTH;
			bool honor = authenticated;
			honorLock.release();
			
			if (honor)
				honorRequest();
		}
		break;
	}
	case S_AWAITING_AUTH:
	{
		// this point shouldn't be reached
		break;
	}
	case S_READING_INIT_DATA:
	{
		ssize_t bytes = fill(srcFD);
		if (bytes == 0)
		{
			deactivate();
			return;
		}
		if (bytes < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				poller->add(this, srcFD, Poller::IN_EVENTS);
			else
				deactivate();
			return;
		}
		if (buf.usedSize() >= req->getInitialDataLen())
		{
			honorLock.acquire();
			state = S_AWAITING_AUTH;
			bool honor = authenticated;
			honorLock.release();
			
			if (honor)
				honorRequest();
		}
		break;
	}
	case S_CONNECTING:
	{
		if ((events & EPOLLOUT) || (events & EPOLLHUP))
		{
			S6U::SocketAddress bindAddr;
			socklen_t addrLen = sizeof(bindAddr.storage);
			int rc = getsockname(dstFD, &bindAddr.sockAddress, &addrLen);
			if (rc < 0)
				throw system_error(errno, system_category());
			
			if (S6U::Socket::hasMPTCP(dstFD) > 0)
				replyOptions.setMPTCP();
				
					
			S6M::OperationReply reply(SOCKS6_OPERATION_REPLY_SUCCESS, bindAddr.getAddress(), bindAddr.getPort(), req->getInitialDataLen(), replyOptions);
			(new ConnectProxyDownstreamer(this, &reply))->start();
			
			state = S_STREAM;
		
			if (buf.usedSize() > 0)
			{
				streamState = SS_WAITING_TO_SEND;
				poller->add(this, dstFD, Poller::OUT_EVENTS);
			}
			else
			{
				streamState = SS_WAITING_TO_RECV;
				poller->add(this, srcFD, Poller::IN_EVENTS);
			}
		}
		else //EPOLLERR
		{
			int err;
			socklen_t errLen = sizeof(err);
			SOCKS6OperationReplyCode code;
			
			int rc = getsockopt(dstFD, SOL_SOCKET, SO_ERROR, &err, &errLen);
			if (rc < 0)
				code = SOCKS6_OPERATION_REPLY_FAILURE;
			else
				code = S6U::Socket::connectErrnoToReplyCode(err);
			
			S6M::OperationReply reply(code, S6M::Address(S6U::Socket::QUAD_ZERO), 0, 0, replyOptions);
			(new SimpleProxyDownstreamer(this, &reply))->start();
		}
		
		break;
	}
	case S_STREAM:
	{
		StreamReactor::process(fd, events);
		break;
	}
	}
}

void ProxyUpstreamer::authDone(SOCKS6TokenExpenditureCode expenditureCode)
{
	if (expenditureCode != (SOCKS6TokenExpenditureCode)0)
		replyOptions.replyToExpenditure(expenditureCode);
	
	honorLock.acquire();
	authenticated = true;
	bool honor = state == S_AWAITING_AUTH;
	honorLock.release();
	
	if (honor)
		honorRequest();
}
