#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <system_error>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "proxifierupstreamer.hh"

#include <iostream>

using namespace std;

void Proxifier::setupReactor(int fd)
{
	ProxifierUpstreamer *upstreamReactor = NULL;
	try
	{
		upstreamReactor = new ProxifierUpstreamer(this, fd);
	}
	catch (...)
	{
		close(fd); // tolerable error
	}
}

Proxifier::~Proxifier()
{
	close(listenFD);
}
