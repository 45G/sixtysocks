#ifndef SOCKET_HH
#define SOCKET_HH

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <system_error>
#include <boost/intrusive_ptr.hpp>
#include "poller.hh"
#include "uniqfd.hh"
#include "tls.hh"

template<typename UFD>
struct Socket
{
	UFD fd;
	boost::intrusive_ptr<TLS> tls;
	
	size_t tcpRecv(StreamBuffer *buf)
	{
		ssize_t bytes = recv(fd, buf->getTail(), buf->availSize(), MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
				throw RescheduleException(fd, Poller::IN_EVENTS);
			throw std::system_error(errno, std::system_category());
		}
		buf->use(bytes);
		return bytes;
	}
	
	size_t tcpSend(StreamBuffer *buf)
	{
		ssize_t bytes = send(fd, buf->getHead(), buf->usedSize(), MSG_NOSIGNAL);
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) //TODO: maybe EINTR as well
				throw RescheduleException(fd, Poller::OUT_EVENTS);
			throw std::system_error(errno, std::system_category());
		}
		buf->unuse(bytes);
		return bytes;
	}
	
	size_t tcpSendTFO(StreamBuffer *buf, size_t maxPayload, S6U::SocketAddress dest)
	{
		ssize_t bytes = sendto(fd, buf->getHead(), std::min(buf->usedSize(), maxPayload), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
		if (bytes < 0 && errno != EINPROGRESS)
			throw std::system_error(errno, std::system_category());
		if (bytes > 0)
			buf->unuse(bytes);
		return bytes;
	}
	
	void tcpConnect(S6U::SocketAddress dest)
	{
		int rc = connect(fd, &dest.sockAddress, dest.size());
		if (rc < 0 && errno != EINPROGRESS)
			throw std::system_error(errno, std::system_category());
	}
	
	void tcpDeferredConnect(S6U::SocketAddress dest)
	{
		static const int ONE = 1;
		int rc = setsockopt(fd, SOL_TCP, TCP_FASTOPEN_CONNECT, ONE, sizeof(ONE));
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		tcpConnect(dest);
	}
	
	size_t sockRecv(StreamBuffer *buf)
	{
		if (tls)
			return tls->tlsRead(buf);
		return tcpRecv(buf);
	}
	
	size_t sockSend(StreamBuffer *buf)
	{
		if (tls)
			return tls->tlsWrite(buf);
		return tcpSend(buf);
	}
	
	void sockConnect(S6U::SocketAddress addr, StreamBuffer *buf, size_t maxTFOPayload, bool earlyDataIfTLS)
	{
		if (tls == nullptr)
		{
			if (maxTFOPayload > 0)
				tcpSendTFO(buf, maxTFOPayload, addr);
			else
				tcpConnect(addr);
		}
		else
		{
			try
			{
				tls->tlsConnect(&addr, buf, earlyDataIfTLS);
			}
			catch (RescheduleException &) {}
		}
	}
	
	int getConnectError()
	{
		/* not our problem */
		if (tls != nullptr)
			return 0;
		
		int err;
		socklen_t errLen = sizeof(err);
		int rc = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
		return err;
	}
	
	bool benefitsFromIdempotence()
	{
		return tls != nullptr;
	}
	
	void duplicate(Socket<UniqSendFD> *ws)
	{
		fd.assign(dup(ws->fd));
		if (fd < 0)
			throw std::system_error(errno, std::system_category());
		tls = ws->tls;
		if (tls != nullptr)
			tls->setReadFD(fd);
	}
	
	void duplicate(Socket<UniqRecvFD> *rs)
	{
		fd.assign(dup(rs->fd));
		if (fd < 0)
			throw std::system_error(errno, std::system_category());
		tls = rs->tls;
		if (tls != nullptr)
			tls->setWriteFD(fd);
	}
	
	void keepAlive()
	{
		static const int ONE = 1;
		int rc = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &ONE, sizeof(ONE));
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
	}
};

typedef Socket<UniqFD> RWSocket;

typedef Socket<UniqSendFD> WSocket;

typedef Socket<UniqRecvFD> RSocket;


#endif // SOCKET_HH
