#include "readabledeferreactor.hh"

#include "../core/poller.hh"

void ReadableDeferReactor::start()
{
	poller->add(this, fd, Poller::IN_EVENTS);
}

void ReadableDeferReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	poller->assign(reactor);
}
