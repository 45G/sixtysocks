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
	enum StreamState
	{
		SS_RECEIVING,
		SS_SENDING,
	};
	
	struct Stream
	{
		RSocket srcSock;
		WSocket dstSock;
		
		StreamBuffer buf;
		
		StreamState state = SS_RECEIVING;
	};
	
	Stream stream;

public:
	FDXStreamReactor(Poller *poller)
		: Reactor(poller){}
	
	void process(int fd, uint32_t events);
	
	void deactivate();

	void start();

	~FDXStreamReactor();

	friend class AuthenticationReactor;
};

#endif // FDXSTREAMREACTOR_HH
