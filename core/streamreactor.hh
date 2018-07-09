#ifndef STREAMREACTOR_HH
#define STREAMREACTOR_HH

#include <socks6util/socks6util_socketaddress.hh>
#include "reactor.hh"

class StreamBuffer
{
	static const size_t BUF_SIZE = 100 * 1024; //100KB
	
	uint8_t buf[BUF_SIZE];
	
	size_t head;
	size_t tail;
	
public:
	StreamBuffer()
		: head(0), tail(0) {}
	
	uint8_t *getHead()
	{
		return &buf[head];
	}
	
	size_t usedSize() const
	{
		return tail - head;
	}
	
	void unuseHead(size_t count)
	{
		head += count;
		if (head == tail)
		{
			head = 0;
			tail = 0;
		}
	}

	void unuseTail(size_t count)
	{
		tail -= count;
		if (head == tail)
		{
			head = 0;
			tail = 0;
		}
	}
	
	uint8_t *getTail()
	{
		return &buf[tail];
	}
	
	size_t availSize() const
	{
		return BUF_SIZE - tail;
	}
	
	void use(size_t count)
	{
		tail += count;
	}
};

class AuthenticationReactor;

class StreamReactor: public Reactor
{
protected:
	int srcFD;
	int dstFD;
	
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

	void resume();

	~StreamReactor();

	friend class AuthenticationReactor;
};

#endif // STREAMREACTOR_HH
