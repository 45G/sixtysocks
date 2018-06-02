#ifndef STREAMREACTOR_HH
#define STREAMREACTOR_HH

#include "reactor.hh"

class StreamReadReactor: public Reactor
{
public:
	StreamReadReactor(int fd);
};

#endif // STREAMREACTOR_HH
