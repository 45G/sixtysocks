#include <sys/socket.h>
#include <errno.h>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <socks6util/socks6util.hh>
#include "poller.hh"
#include "listenreactor.hh"

using namespace std;

ListenReactor::ListenReactor(Poller *poller, const S6U::SocketAddress &bindAddr)
	: Reactor(poller)
{
	listenFD.assign(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
	if (listenFD < 0)
		throw system_error(errno, system_category());

	// tolerable error
	static const int ONE = 1;
	setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &ONE, sizeof(ONE));

	int rc = ::bind(listenFD, &bindAddr.sockAddress, bindAddr.size());
	if (rc < 0)
		throw system_error(errno, system_category());
	
	const static int TFO_QLEN = 256; //TODO: configurable queue length?
	setsockopt(listenFD, SOL_TCP, TCP_FASTOPEN, &TFO_QLEN, sizeof(TFO_QLEN)); // tolerable error

	const static int BACKLOG = 128; //TODO: configurable backlog?
	rc = listen(listenFD, BACKLOG);
	if (rc < 0)
		throw system_error(errno, system_category());
}

void ListenReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;
	
	while (isActive())
	{
		int clientFD = accept4(listenFD, nullptr, nullptr, SOCK_NONBLOCK);
		if (clientFD < 0)
		{
			switch (errno)
			{
			case EWOULDBLOCK:
#if EWOULDBLOCK != EAGAIN
			case EAGAIN:
#endif
				goto resched;

			case EINTR:
			case ENETDOWN:
			case EPROTO:
			case ENOPROTOOPT:
			case EHOSTDOWN:
			case ENONET:
			case EHOSTUNREACH:
			case EOPNOTSUPP:
			case ENETUNREACH:
			case ECONNABORTED:
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
				break;
				
			default:
				throw system_error(errno, system_category());
			}
			continue;
		}
		
		static const int ONE = 1;
		setsockopt(clientFD, SOL_TCP, TCP_NODELAY, &ONE, sizeof(ONE)); // tolerable error

		handleNewConnection(clientFD);		
resched:
		poller->add(this, listenFD, EPOLLIN);
		break;
	}
}

void ListenReactor::deactivate()
{
	Reactor::deactivate();
	poller->remove(listenFD);
}

void ListenReactor::start()
{
	poller->add(this, listenFD, EPOLLIN);
}

ListenReactor::~ListenReactor()
{
	poller->remove(listenFD);
}
