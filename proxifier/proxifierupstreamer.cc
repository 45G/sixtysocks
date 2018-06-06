#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "../core/poller.hh"
#include "proxifier.hh"
#include "proxifierdownstreamer.hh"
#include "proxifierupstreamer.hh"

using namespace std;

void ProxifierUpstreamer::process(Poller *poller, uint32_t events)
{
	if (!active)
		return;
	
	switch (state)
	{
	case S_READING_INIT_DATA:
	{
		S6U::SocketAddress dest;
		S6U::Socket::getOriginalDestination(srcFD, &dest.storage);
		
		S6M::OptionSet opts(S6M::OptionSet::M_REQ);
		if (S6U::Socket::tfoAttempted(srcFD))
			opts.setTFO();
		S6M::Request req(SOCKS6_REQUEST_CONNECT, dest.getAddress(), dest.getPort(), opts, 0);
		S6M::ByteBuffer bb(buf.getTail(), buf.freeSize());
		req.pack(&bb);
		buf.use(bb.getUsed());
		reqBytesLeft = bb.getUsed();
		
		ssize_t bytes = recv(srcFD, buf.getTail(), buf.freeSize(), MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.use(bytes);
		
		dstFD = socket(owner->getProxy()->storage.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (dstFD < 0)
			throw system_error(errno, system_category());
		
		int rc = fcntl(dstFD, F_SETFD, O_NONBLOCK);
		if (rc < 0)
			throw system_error(errno, std::system_category());
		
		state = S_SENDING_REQ;
		
		//TODO: check if TFO is wanted
		bytes = sendto(dstFD, buf.getHead(), buf.usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
		if (bytes < 0)
		{
			if (errno != EINPROGRESS)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.free(bytes);
		reqBytesLeft -= bytes;
		
		if (reqBytesLeft < 0)
		{
			downstreamer = new ProxifierDownstreamer(this);
			downstreamer->use();
			poller->add(downstreamer, downstreamer->getSrcFD(), EPOLLIN | EPOLLRDHUP);
			state = buf.usedSize() > 0 ? S_WAITING_TO_SEND : S_WAITING_TO_RECV;
		}
			
		if (state == S_SENDING_REQ || state == S_WAITING_TO_SEND)
			poller->add(this, dstFD, EPOLLOUT);
		else
			poller->add(this, srcFD, EPOLLIN);
		
		break;
	}
	case S_SENDING_REQ:
	{
		ssize_t bytes = send(dstFD, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
		if (bytes == 0)
			return;
		if (bytes < 0)
		{
			if (errno != EINPROGRESS)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.free(bytes);
		reqBytesLeft -= bytes;
		
		if (reqBytesLeft < 0)
		{
			downstreamer = new ProxifierDownstreamer(this);
			downstreamer->use();
			poller->add(downstreamer, downstreamer->getSrcFD(), EPOLLIN | EPOLLRDHUP);
			state = buf.usedSize() > 0 ? S_WAITING_TO_SEND : S_WAITING_TO_RECV;
		}
			
		if (state == S_SENDING_REQ || state == S_WAITING_TO_SEND)
			poller->add(this, dstFD, EPOLLOUT);
		else
			poller->add(this, srcFD, EPOLLIN);
		
		break;
	}
	case S_WAITING_TO_RECV:
	{
		ssize_t bytes = recv(srcFD, buf.getTail(), buf.freeSize(), MSG_NOSIGNAL);
		if (bytes == 0)
		{
			close(srcFD); // tolerable error
			srcFD = -1;
		}
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				close(srcFD); // tolerable error
				srcFD = -1;
			}
			bytes = 0;
		}
		buf.use(bytes);
		
		bytes = send(dstFD, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.free(bytes);
		
		if (buf.usedSize() > 0)
		{
			state = S_WAITING_TO_SEND;
			poller->add(this, dstFD, EPOLLOUT);
		}
		else
		{
			poller->add(this, srcFD, EPOLLIN | EPOLLRDHUP);
		}
		
		break;
	}
	case S_WAITING_TO_SEND:
	{
		
		break;
	}
	}
	
}

int ProxifierUpstreamer::getFD() const
{
	switch (state)
	{
	case S_READING_INIT_DATA:
	case S_WAITING_TO_RECV:
		return srcFD;
		
	case S_SENDING_REQ:
	case S_WAITING_TO_SEND:
		return dstFD;
	}
	
	return -1;
}
