#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include "../core/streamreactor.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_INIT_DATA,
		S_SENDING_REQ,
		S_STREAM,
	};
	
	ssize_t reqBytesLeft;
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state;
	
public:
	ProxifierUpstreamer(Proxifier *proxifier, int srcFD);

	void process(int fd, uint32_t events);
	
	Proxifier *getProxifier()
	{
		return proxifier.get();
	}
};

#endif // PROXIFIERUPSTREAMER_HH
