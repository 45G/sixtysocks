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
	
	int fill(int fd);

	int spill(int fd);

	int spillTFO(int fd, S6U::SocketAddress dest);

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
