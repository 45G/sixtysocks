#include <socks6util/socks6util.hh>
#include <socks6msg/socks6msg.hh>
#include <system_error>
#include <fcntl.h>
#include "proxifier.hh"
#include "proxiferupstreamer.hh"

using namespace std;

void ProxiferUpstreamer::process(Poller *poller, uint32_t events)
{
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
		
		//TODO: check if TFO is wanted
		bytes = sendto(dstFD, buf.getHead(), buf.usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.storage, dest.size());
		bytes = S6U::Socket::fastOpenConnect(dstFD, &dest.storage, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno != EINPROGRESS)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.free(bytes);
		
		break;
	}
	case S_WAITING_FOR_DATA:
	{
		break;
	}
	case S_CONNECTING:
	{
		break;
	}
	case S_SENDING_REQ:
	{
		break;
	}
	case S_WAITING_TO_SEND:
	{
		break;
	}
	}
	
}

int ProxiferUpstreamer::getFD() const
{
	switch (state)
	{
	case S_READING_INIT_DATA:
	case S_WAITING_FOR_DATA:
		return srcFD;
	
	case S_CONNECTING:
		return -1;
		
	case S_SENDING_REQ:
	case S_WAITING_TO_SEND:
		return dstFD;
	}
	
	return -1;
}
