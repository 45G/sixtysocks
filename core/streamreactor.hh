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
		SS_RECEIVING,
		SS_SENDING,
	};

	StreamState streamState;

public:
	StreamReactor(Poller *poller, StreamState streamState = SS_RECEIVING)
		: Reactor(poller), streamState(streamState) {}
	
	UniqRecvFD *getSrcFD()
	{
		return &srcFD;
	}
	
	UniqSendFD *getDstFD()
	{
		return &dstFD;
	}
	
	void process(int fd, uint32_t events);
	
	void deactivate();

	void start();

	~StreamReactor();

	friend class AuthenticationReactor;
};

#endif // STREAMREACTOR_HH
