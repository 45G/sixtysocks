#ifndef FDXSTREAMREACTOR_HH
#define FDXSTREAMREACTOR_HH

#include <socks6util/socketaddress.hh>
#include "streambuffer.hh"
#include "socket.hh"
#include "reactor.hh"

class AuthenticationReactor;

class FDXStreamReactor: public Reactor
{
protected:
	RSocket srcSock;
	WSocket dstSock;
	
	StreamBuffer buf;
	
	enum StreamState
	{
		SS_RECEIVING,
		SS_SENDING,
	};

	StreamState streamState = SS_RECEIVING;

public:
	FDXStreamReactor(Poller *poller)
		: Reactor(poller){}
	
	RSocket *getSrcSock()
	{
		return &srcSock;
	}
	
	WSocket *getDstSock()
	{
		return &dstSock;
	}
	
	void process(int fd, uint32_t events);
	
	void deactivate();

	void start();

	~FDXStreamReactor();

	friend class AuthenticationReactor;
};

#endif // FDXSTREAMREACTOR_HH
