#ifndef TFOCOOKIESUPPLICATIONAGENT_HH
#define TFOCOOKIESUPPLICATIONAGENT_HH

#include "../core/reactor.hh"
#include "../core/uniqfd.hh"

class Proxifier;

class TFOCookieSupplicationAgent: public Reactor
{
	boost::intrusive_ptr<Proxifier> proxifier;

	UniqFD sock;

	StreamBuffer buf;

public:
	TFOCookieSupplicationAgent(Proxifier *proxifier);

	void start();

	void process(int fd, uint32_t events);

	void deactivate();
};

#endif // TFOCOOKIESUPPLICATIONAGENT_HH
