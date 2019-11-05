#ifndef READABLEDEFERREACTOR_HH
#define READABLEDEFERREACTOR_HH

#include "../core/reactor.hh"

class ReadableDeferReactor: public Reactor
{
	int fd;
	boost::intrusive_ptr<Reactor> reactor;

public:
	ReadableDeferReactor(Poller *poller, int fd, boost::intrusive_ptr<Reactor> reactor)
		: Reactor(poller), fd(fd), reactor(reactor) {}

	void start();

	void process(int fd, uint32_t events);
};

#endif // READABLEDEFERREACTOR_HH
