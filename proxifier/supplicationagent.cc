#include <system_error>
#include <socks6msg/socks6msg.hh>
#include "proxifier.hh"
#include "supplicationagent.hh"
#include "../core/poller.hh"

using namespace std;

SupplicationAgent::SupplicationAgent(Proxifier *proxifier)
	: Reactor(proxifier->getPoller()), proxifier(proxifier), state(S_SENDING_REQ), supplicating(true)
{
	const S6U::SocketAddress *proxyAddr = proxifier->getProxyAddr();
	fd = socket(proxyAddr->sockAddress.sa_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd < 0)
		throw system_error(errno, system_category());
	
	int rc = connect(fd, &proxyAddr->sockAddress, proxyAddr->size());
	if (rc < 0 && errno != EINPROGRESS)
		throw system_error(errno, system_category());
	
	S6M::Request req(SOCKS6_REQUEST_NOOP, S6U::Socket::QUAD_ZERO, 0, 0);
	req.getOptionSet()->setUsernamePassword(proxifier->getUsername(), proxifier->getPassword());
	req.getOptionSet()->requestTokenWindow(200); //TODO: make configurable

	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
	req.pack(&bb);
	buf.use(bb.getUsed());
}

SupplicationAgent::~SupplicationAgent()
{
	if (supplicating)
		proxifier->supplicantDone();
}

void SupplicationAgent::process(int fd, uint32_t events)
{
	(void)events;
	
	switch (state)
	{
	case S_SENDING_REQ:
	{
		ssize_t bytes = send(fd, buf.getHead(), buf.usedSize(), MSG_NOSIGNAL);
		if (bytes == 0)
			return;
		if (bytes < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw system_error(errno, system_category());
			bytes = 0;
		}
		buf.unuseHead(bytes);
		
		if (buf.usedSize() > 0)
		{
			poller->add(this, fd, Poller::OUT_EVENTS);
		}
		else
		{
			state = S_RECEIVING_AUTH_REP;
			poller->add(this, fd, Poller::IN_EVENTS);
		}
		break;
	}
		
	case S_RECEIVING_AUTH_REP:
	{
		ssize_t bytes = recv(fd, buf.getTail(), buf.availSize(), MSG_NOSIGNAL);
		if (bytes == 0)
			return;
		if (bytes < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw system_error(errno, system_category());
			poller->add(this, fd, Poller::IN_EVENTS);
			return;
		}
		buf.use(bytes);
		
		try
		{
			S6M::ByteBuffer bb(buf.getHead(), buf.usedSize());
			S6M::AuthenticationReply authRep(&bb);
			
			uint32_t size = authRep.getOptionSet()->getTokenWindowSize();
			if (size > 0)
			{
				uint32_t base = authRep.getOptionSet()->getTokenWindowBase();
				proxifier->setWallet(boost::shared_ptr<S6U::TokenWallet>(new S6U::TokenWallet(base, size)));
			}
			
			supplicating = false;
			proxifier->supplicantDone();
			
		}
		catch (S6M::EndOfBufferException)
		{
			poller->add(this, fd, Poller::IN_EVENTS);
		}
		break;
	}
	}
}

void SupplicationAgent::deactivate()
{
	Reactor::deactivate();
	poller->remove(fd);
	close(fd); //tolerable error
}

void SupplicationAgent::start(bool defer)
{
	poller->add(this, fd, Poller::OUT_EVENTS);
}
