#include <system_error>
#include <socks6util/socks6util.hh>
#include "../core/poller.hh"
#include "proxy.hh"
#include "authserver.hh"
#include "connectproxydownstreamer.hh"
#include "simpleproxydownstreamer.hh"
#include "proxyupstreamer.hh"

using namespace std;

void ProxyUpstreamer::honorRequest()
{
	try
	{
		switch (request->code)
		{
		case SOCKS6_REQUEST_CONNECT:
		{
			honorConnect();
			break;
		}
		case SOCKS6_REQUEST_NOOP:
		{
			throw SimpleReplyException(SOCKS6_OPERATION_REPLY_SUCCESS);
		}
		default:
		{
			throw SimpleReplyException(SOCKS6_OPERATION_REPLY_CMD_NOT_SUPPORTED);
		}
		}
	}
	catch (SimpleReplyException &ex)
	{
		reply.code = ex.getCode();
		poller->assign(new SimpleProxyDownstreamer(this, &reply));
	}
	catch (std::exception &)
	{
		reply.code = SOCKS6_OPERATION_REPLY_FAILURE;
		poller->assign(new SimpleProxyDownstreamer(this, &reply));
	}
}

void ProxyUpstreamer::honorConnect()
{
	//TODO: resolve
	if (request->address.getType() == SOCKS6_ADDR_DOMAIN)
		throw SimpleReplyException(SOCKS6_OPERATION_REPLY_ADDR_NOT_SUPPORTED);

	/* resolve zero to loopback for port DNS port */
	addr = [&]() {
		if (request->address.isZero() && request->port == 53)
			return S6M::Address(in_addr{ INADDR_LOOPBACK });
		return request->address;
	} ();
	
	S6U::SocketAddress sockAddr(addr, request->port);
		
	dstSock.fd.assign(socket(sockAddr.sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (dstSock.fd < 0)
		throw system_error(errno, system_category());

	honorConnectStackOptions();

	try
	{
		dstSock.sockConnect(sockAddr, &buf, request->options.stack.tfo.get(SOCKS6_STACK_LEG_PROXY_REMOTE).value_or(0), false);
	}
	catch (system_error &err)
	{
		throw SimpleReplyException(S6U::Socket::connectErrnoToReplyCode(err.code().value()));
	}

	poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
	state = S_CONNECTING;
}

void ProxyUpstreamer::honorConnectStackOptions()
{
	//TODO
}

ProxyUpstreamer::ProxyUpstreamer(Proxy *proxy, UniqFD &&srcFD)
	: StreamReactor(proxy->getPoller()), proxy(proxy)
{
	srcSock.fd = move(srcFD);
	srcSock.keepAlive();
	
	TLSContext *serverCtx = proxy->getServerCtx();
	if (serverCtx)
		srcSock.tls = new TLS(serverCtx, srcSock.fd);
}

void ProxyUpstreamer::start()
{
	process(-1, 0);
}

void ProxyUpstreamer::process(int fd, uint32_t events)
{
	switch ((State)state)
	{
	case S_READING_REQ:
	{
		ssize_t bytes = srcSock.sockRecv(&buf);
		if (bytes == 0)
			return;

		S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
		try
		{
			request = std::make_shared<S6M::Request>(&bb);
			buf.unuse(bb.getUsed());
		}
		catch (S6M::BadVersionException &)
		{
			SOCKS6Version version = { SOCKS6_VERSION };
			poller->assign(new SimpleProxyDownstreamer(this, &version));
			return;
		}
		catch (S6M::EndOfBufferException &)
		{
			poller->add(this, srcSock.fd, Poller::IN_EVENTS);
			return;
		}

		poller->assign(new AuthServer(this));

		tfoPayload = std::min((size_t)request->options.stack.tfo.get().value_or(0), MSS);
		
		if (buf.usedSize() < tfoPayload)
		{
			state = S_READING_TFO_PAYLOAD;
			poller->add(this, srcSock.fd, Poller::IN_EVENTS);
		}
		else
		{
			honorLock.lock();
			state = S_AWAITING_AUTH;
			bool honor = authenticated;
			honorLock.unlock();
			
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
	case S_READING_TFO_PAYLOAD:
	{
		ssize_t bytes = srcSock.sockRecv(&buf);
		if (bytes == 0)
		{
			deactivate();
			return;
		}
		if (buf.usedSize() >= tfoPayload)
		{
			honorLock.lock();
			state = S_AWAITING_AUTH;
			bool honor = authenticated;
			honorLock.unlock();
			
			if (honor)
				honorRequest();
		}
		break;
	}
	case S_CONNECTING:
	{
		try
		{
			reply.code = S6U::Socket::connectErrnoToReplyCode(dstSock.getConnectError());
		}
		catch (system_error &)
		{
			reply.code = SOCKS6_OPERATION_REPLY_FAILURE;
		}

		if (reply.code != SOCKS6_OPERATION_REPLY_SUCCESS)
		{
			poller->assign(new SimpleProxyDownstreamer(this, &reply));
			return;
		}
		
		dstSock.keepAlive();

		S6U::SocketAddress bindAddr;
		socklen_t addrLen = sizeof(bindAddr.storage);
		int rc = getsockname(dstSock.fd, &bindAddr.sockAddress, &addrLen);
		if (rc < 0)
			throw system_error(errno, system_category());

		if (S6U::Socket::hasMPTCP(dstSock.fd) > 0)
			reply.options.stack.mp.set(SOCKS6_STACK_LEG_PROXY_REMOTE, SOCKS6_MP_AVAILABLE);

		reply.code = SOCKS6_OPERATION_REPLY_SUCCESS;
		reply.address = bindAddr.getAddress();
		reply.port = bindAddr.getPort();
		poller->assign(new ConnectProxyDownstreamer(this, &reply));

		state = S_STREAM;
		if (buf.usedSize() > 0)
			streamState = SS_SENDING;
		else
			streamState = SS_RECEIVING;
		[[fallthrough]];
	}
	case S_STREAM:
	{
		StreamReactor::process(fd, events);
		break;
	}
	}
}

void ProxyUpstreamer::authDone()
{
	honorLock.lock();
	authenticated = true;
	bool honor = state == S_AWAITING_AUTH;
	honorLock.unlock();
	
	if (honor)
		honorRequest();
}
