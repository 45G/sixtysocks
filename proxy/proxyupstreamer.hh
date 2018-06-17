#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include "core/streamreactor.hh"

class ProxyUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_REQ,
		S_STREAM,
	};
public:
	ProxyUpstreamer(Proxifier *owner, int srcFD);
};

#endif // PROXYUPSTREAMER_HH
