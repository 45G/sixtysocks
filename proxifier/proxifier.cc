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
#include <boost/intrusive_ptr.hpp>
#include "proxifier.hh"
#include "../core/poller.hh"
#include "../proxifier/readabledeferreactor.hh"
#include "sessionsupplicationagent.hh"
#include "tfocookiesupplicationagent.hh"
#include "proxifierupstreamer.hh"

using namespace std;
using boost::intrusive_ptr;

Proxifier::Proxifier(Poller *poller, const S6U::SocketAddress &proxyAddr, const S6U::SocketAddress &bindAddr, bool defer, const pair<string_view, string_view> &credentials, TLSContext *clientCtx)
	: ListenReactor(poller, bindAddr), proxyAddr(proxyAddr), defer(defer),
	  username(credentials.first), password(credentials.second),
	  clientCtx(clientCtx)
{
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
}

void Proxifier::start()
{
	bool supplicateTFO = true;

	supplicationLock.lock();
	try
	{
		auto sessionSupplicant = make_shared<SessionSupplicant>(this);
		poller->assign(new SessionSupplicationAgent(this, sessionSupplicant, clientCtx));

		if (clientCtx) /* TLS uses TFO */
			supplicateTFO = false;
	}
	catch (exception &ex)
	{
		cerr << "Error supplicating token window: " << ex.what() << endl;
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
	UniqFD ufd(fd);
	std::shared_ptr<SessionSupplicant> supplicant;

	if (supplicationLock.try_lock())
	{
		if (!session)
			supplicant = make_shared<SessionSupplicant>(this);
		else
			supplicationLock.unlock();
	}
	
	try
	{

		intrusive_ptr<ProxifierUpstreamer> upstreamer = new ProxifierUpstreamer(this, move(ufd), supplicant);
		if (defer)
			poller->assign(new ReadableDeferReactor(poller, fd, upstreamer));
		else
			poller->assign(upstreamer);
	}
	catch (exception &ex)
	{
		cerr << "Error handling new connection: " << ex.what() << endl;
	}
}
