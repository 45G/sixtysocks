#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <system_error>
#include <iostream>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "../core/readabledeferreactor.hh"
#include "windowsupplicationagent.hh"
#include "tfocookiesupplicationagent.hh"
#include "proxifierupstreamer.hh"

using namespace std;

Proxifier::Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, bool defer, const string &username, const string &password, TLSContext *clientCtx)
	: ListenReactor(poller, bindAddr), proxyAddr(proxyAddr), defer(defer),
	  username(username), password(password),
	  wallet(new SyncedTokenWallet()),
	  clientCtx(clientCtx)
{
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
}

void Proxifier::start()
{
	bool supplicateTFO = true;

	supplicationLock.lock();
	if (username.length() > 0)
	{
		try
		{
			std::shared_ptr<SessionSupplicant> windowSupplicant (new SessionSupplicant(this));
			poller->assign(new WindowSupplicationAgent(this, windowSupplicant, clientCtx));

			if (clientCtx != nullptr) /* TLS uses TFO */
				supplicateTFO = false;
		}
		catch (exception &ex)
		{
			cerr << "Error supplicating token window: " << ex.what() << endl;
		}
	}

	if (supplicateTFO)
	{
		try
		{
			poller->assign(new TFOCookieSupplicationAgent(this));
		}
		catch (exception &ex)
		{
			cerr << "Error supplicating TFO cookie: " << ex.what() << endl;
		}
	}

	ListenReactor::start();
}

void Proxifier::handleNewConnection(int fd)
{
	std::shared_ptr<SessionSupplicant> supplicant;

	if (supplicationLock.try_lock())
	{
		if (wallet->remaining() == 0)
			supplicant = std::shared_ptr<SessionSupplicant>(new SessionSupplicant(this));
		else
			supplicationLock.unlock();
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
	catch (exception &ex)
	{
		cout << "Error handling new connection: " << ex.what() << endl;
	}

	if (closeFD != -1)
		close(closeFD); // tolerable error
}
