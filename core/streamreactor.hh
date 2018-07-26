#ifndef STREAMREACTOR_HH
#define STREAMREACTOR_HH

#include <socks6util/socks6util_socketaddress.hh>
#include "streambuffer.hh"
#include "uniqfd.hh"
#include "reactor.hh"

class AuthenticationReactor;

class StreamReactor: public Reactor
{
protected:
	UniqRecvFD srcFD;
	UniqSendFD dstFD;
	
	StreamBuffer buf;
	
	enum StreamState
	{
		SS_WAITING_TO_RECV,
		SS_WAITING_TO_SEND,
	};

	StreamState streamState;
	
	int fill(int fd)
	{
		ssize_t bytes = recv(fd, buf.getTail(), buf.availSize(), MSG_NOSIGNAL);
		if (bytes > 0)
			buf.use(bytes);
		return bytes;
	}

	int spill(int fd)
	{
		ssize_t bytes = send(fd, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
		if (bytes >0)
			buf.unuseHead(bytes);
		return bytes;
	}

	int spillTFO(int fd, S6U::SocketAddress dest)
	{
		ssize_t bytes = sendto(fd, buf.getHead(), buf.usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL, &dest.sockAddress, dest.size());
		if (bytes > 0)
			buf.unuseHead(bytes);
		return bytes;
	}

public:
	StreamReactor(Poller *poller, int srcFD, int dstFD, StreamState streamState = SS_WAITING_TO_RECV)
		: Reactor(poller), srcFD(srcFD), dstFD(dstFD), streamState(streamState) {}
	
	int getSrcFD() const
	{
		return srcFD;
	}
	
	int getDstFD() const
	{
		return dstFD;
	}
	
	void process(int fd, uint32_t events);
	
	void deactivate();

	void start(bool defer = false);

	~StreamReactor();

	friend class AuthenticationReactor;
};

#endif // STREAMREACTOR_HH
