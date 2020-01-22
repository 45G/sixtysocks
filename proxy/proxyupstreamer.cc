#include <system_error>
#include <socks6util/socks6util.hh>
#include "../core/poller.hh"
#include "proxy.hh"
#include "authserver.hh"
#include "connectproxydownstreamer.hh"
#include "simpleproxydownstreamer.hh"
#include "proxyupstreamer.hh"

using namespace std;

void ProxyUpstreamer::addrFixupAndHonorRequest()
{
	if (request->address.getType() == SOCKS6_ADDR_DOMAIN)
	{
		//proxy->getResolver()->resolve(this, *addr.getDomain());
		reply.code = SOCKS6_OPERATION_REPLY_ADDR_NOT_SUPPORTED;
		poller->assign(new SimpleProxyDownstreamer(this, &reply));
		return;
	}
	
	/* redirect default services locally */
	if (request->address.isZero() && Proxy::DEFAULT_SERVICES.find(request->port) != Proxy::DEFAULT_SERVICES.end())
		addr = S6M::Address(in_addr{ INADDR_LOOPBACK });
	else
		addr = request->address;
	
	honorRequest();
}

void ProxyUpstreamer::honorRequest()
{
	try
	{
		switch (request->code)
		{
		case SOCKS6_REQUEST_CONNECT:
			honorConnect();
			break;
			
		case SOCKS6_REQUEST_NOOP:
			reply.code = SOCKS6_OPERATION_REPLY_SUCCESS;
			poller->assign(new SimpleProxyDownstreamer(this, &reply));
			break;
			
		default:
			reply.code = SOCKS6_OPERATION_REPLY_CMD_NOT_SUPPORTED;
			poller->assign(new SimpleProxyDownstreamer(this, &reply));
			break;
		}
	}
	catch (std::exception &)
	{
		reply.code = SOCKS6_OPERATION_REPLY_FAILURE;
		poller->assign(new SimpleProxyDownstreamer(this, &reply));
	}
}

void ProxyUpstreamer::honorConnect()
{
	S6U::SocketAddress sockAddr(addr, request->port);
		
	dstSock.fd.assign(socket(sockAddr.sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP));
	if (dstSock.fd < 0)
		throw system_error(errno, system_category());

	honorConnectStackOptions();
	
	state = S_READING_TFO_PAYLOAD;
	process(-1, 0);	
}

void ProxyUpstreamer::honorConnectStackOptions()
{
	tfoPayload = std::min((size_t)request->options.stack.tfo.get().value_or(0), MSS);
}

void ProxyUpstreamer::populateConnectStackOptions()
{
	if (S6U::Socket::hasMPTCP(dstSock.fd) > 0)
		reply.options.stack.mp.set(SOCKS6_STACK_LEG_PROXY_REMOTE, SOCKS6_MP_AVAILABLE);
}

ProxyUpstreamer::ProxyUpstreamer(Proxy *proxy, UniqFD &&srcFD)
	: StreamReactor(proxy->getPoller()), proxy(proxy)
{
	srcSock.fd = move(srcFD);
	srcSock.keepAlive();
	
	TLSContext *serverCtx = proxy->getServerCtx();
	if (serverCtx)
		srcSock.tls = make_shared<TLS>(serverCtx, srcSock.fd);
}

void ProxyUpstreamer::start()
{
	//proxy->getTimeoutReactor()->add(&timer);
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
		break;
	}
		
	case S_READING_TFO_PAYLOAD:
	{
		//TODO: get rid of if (need extra state in enum)
		if (buf.usedSize() < tfoPayload)
		{
			ssize_t bytes = srcSock.sockRecv(&buf);
			if (bytes == 0)
				return;
			if (buf.usedSize() < tfoPayload)
			{
				poller->add(this, srcSock.fd, Poller::IN_EVENTS);
				return;
			}
		}

		//timer.refresh();
		
		S6U::SocketAddress sockAddr(addr, request->port);
		try
		{
			dstSock.sockConnect(sockAddr, &buf, tfoPayload, false);
		}
		catch (system_error &)
		{
			//TODO: maybe check error code?
			reply.code = SOCKS6_OPERATION_REPLY_FAILURE;
			poller->assign(new SimpleProxyDownstreamer(this, &reply));
			return;
		}
	
		poller->add(this, dstSock.fd, Poller::OUT_EVENTS);
		state = S_CONNECTING;
		
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

		reply.code = SOCKS6_OPERATION_REPLY_SUCCESS;
		reply.address = bindAddr.getAddress();
		reply.port = bindAddr.getPort();
		
		populateConnectStackOptions();
		
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
		//timer.refresh();
		StreamReactor::process(fd, events);
		break;
	}
	}
}

void ProxyUpstreamer::resolvDone(optional<S6M::Address> resolved)
{
	if (!resolved)
	{
		reply.code = SOCKS6_OPERATION_REPLY_HOST_UNREACH;
		poller->assign(new SimpleProxyDownstreamer(this, &reply));
		return;
	}
	
	addr = *resolved;
	honorRequest();
}
