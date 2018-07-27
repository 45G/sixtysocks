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
#include "supplicationagent.hh"
#include "proxifierupstreamer.hh"

using namespace std;

Proxifier::Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, const string &username, const string &password)
	: ListenReactor(poller, bindAddr), proxyAddr(proxyAddr),
	  username(new std::string(username)), password(new std::string(password)),
	  wallet(new LockableTokenWallet())
{
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
}

void Proxifier::start(bool defer)
{
	ListenReactor::start(defer);
	supplicationLock.acquire();
	if (username->length() > 0)
	{
		(new SupplicationAgent(this, boost::shared_ptr<WindowSupplicant>(new WindowSupplicant(this))))->start();
	}
}

void Proxifier::handleNewConnection(int fd)
{
	ProxifierUpstreamer *upstreamReactor = NULL;
	boost::shared_ptr<WindowSupplicant> supplicant;

	if (supplicationLock.attempt())
	{
		if (wallet->remaining() == 0)
			supplicant = boost::shared_ptr<WindowSupplicant>(new WindowSupplicant(this));
		else
			supplicationLock.release();
	}

	try
	{

		upstreamReactor = new ProxifierUpstreamer(this, fd, supplicant);
	}
	catch (...)
	{
		close(fd); // tolerable error
		return;
	}
	
	upstreamReactor->start(true);
}
