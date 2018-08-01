#ifndef STREAMREACTORNG_HH
#define STREAMREACTORNG_HH

#include "streambuffer.hh"
#include "uniqfd.hh"
#include "reactor.hh"

class FSM;

class StreamReactorNG: public Reactor
{
public:
	StreamReactorNG();
};

#endif // STREAMREACTORNG_HH
