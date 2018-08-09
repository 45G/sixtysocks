#ifndef SOCKET_HH
#define SOCKET_HH

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
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
	
	size_t tcpSendTFO(StreamBuffer *buf, S6U::SocketAddress dest)
	{
		ssize_t bytes = sendto(fd, buf->getHead(), buf->usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
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
	
	size_t sockRecv(StreamBuffer *buf)
	{
		if (tls)
			tls->tlsRead(buf);
		return tcpRecv(buf);
	}
	
	size_t sockSend(StreamBuffer *buf)
	{
		if (tls)
			tls->tlsWrite(buf);
		return tcpSend(buf);
	}
	
	void sockConnect(S6U::SocketAddress addr, StreamBuffer *buf, bool tfoIfTCP, bool earlyDataIfTLS)
	{
		if (tls == NULL)
		{
			if (tfoIfTCP)
				tcpSendTFO(buf, addr);
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
	
	void clientHandshake()
	{
		if (tls == NULL)
			return;
		
		tls->tlsConnect(NULL, NULL, false);
	}
	
	void serverHandshake(StreamBuffer *buf)
	{
		if (tls == NULL)
			return;
		
		tls->tlsAccept(buf);
	}
	
	bool benefitsFromIdempotence()
	{
		return tls != NULL;
	}
};

typedef Socket<UniqFD> RWSocket;

typedef Socket<UniqSendFD> WSocket;

typedef Socket<UniqRecvFD> RSocket;

#endif // SOCKET_HH
