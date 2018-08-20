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
#include "../core/readabledeferreactor.hh"
#include "windowsupplicationagent.hh"
#include "tfocookiesupplicationagent.hh"
#include "proxifierupstreamer.hh"

using namespace std;

Proxifier::Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, bool defer, const string &username, const string &password, TLSContext *clientCtx)
	: ListenReactor(poller, bindAddr), proxyAddr(proxyAddr), defer(defer),
	  username(new std::string(username)), password(new std::string(password)),
	  wallet(new SyncedTokenWallet()),
	  clientCtx(clientCtx)
{
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
}

void Proxifier::start()
{
	bool supplicateTFO = true;

	supplicationLock.acquire();
	if (username->length() > 0)
	{
		try
		{
			boost::shared_ptr<WindowSupplicant> windowSupplicant (new WindowSupplicant(this));
			poller->assign(new WindowSupplicationAgent(this, windowSupplicant, clientCtx));

			if (clientCtx != NULL) /* TLS uses TFO */
				supplicateTFO = false;
		}
		catch(...) {}
	}

	if (supplicateTFO)
	{
		try
		{
			poller->assign(new TFOCookieSupplicationAgent(this));
		}
		catch(...) {}
	}

	ListenReactor::start();
}

void Proxifier::handleNewConnection(int fd)
{
	boost::shared_ptr<WindowSupplicant> supplicant;

	if (supplicationLock.attempt())
	{
		if (wallet->remaining() == 0)
			supplicant = boost::shared_ptr<WindowSupplicant>(new WindowSupplicant(this));
		else
			supplicationLock.release();
	}

	int closeFD = fd;
	try
	{

		ProxifierUpstreamer *upstreamer = new ProxifierUpstreamer(this, &closeFD, clientCtx, supplicant);
		if (defer)
			poller->assign(new ReadableDeferReactor(poller, fd, upstreamer));
		else
			poller->assign(upstreamer);
	}
	catch (...)
	{
		close(closeFD); // tolerable error
	}
}
