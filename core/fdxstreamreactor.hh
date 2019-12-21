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
	
	Stream upStream;
	Stream downStream;
	
	void streamProcess(Stream *stream, int fd, uint32_t events);
	
	virtual void upStreamProcess(  int fd, uint32_t events);
	virtual void downStreamProcess(int fd, uint32_t events);

public:
	FDXStreamReactor(Poller *poller)
		: Reactor(poller){}
	
	void process(int fd, uint32_t events);
	
	void deactivate();

	void start();
	
	void upStreamStart();
	
	void downStreamStart();

	~FDXStreamReactor();

	friend class AuthenticationReactor;
};

#endif // FDXSTREAMREACTOR_HH
