#ifndef TFOCOOKIESUPPLICATIONAGENT_HH
#define TFOCOOKIESUPPLICATIONAGENT_HH

#include "../core/stickreactor.hh"

class Proxifier;

class TFOCookieSupplicationAgent: public StickReactor
{
	boost::intrusive_ptr<Proxifier> proxifier;

public:
	TFOCookieSupplicationAgent(Proxifier *proxifier);

	void start();

	void process(int fd, uint32_t events);
};

#endif // TFOCOOKIESUPPLICATIONAGENT_HH
