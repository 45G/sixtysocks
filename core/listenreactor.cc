#include <sys/socket.h>
#include <errno.h>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <socks6util/socks6util.hh>
#include "poller.hh"
#include "listenreactor.hh"

ListenReactor::ListenReactor(Poller *poller, const S6U::SocketAddress &bindAddr)
	: Reactor(poller)
{
	static const int ONE = 1;

	listenFD.assign(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
	if (listenFD < 0)
		throw std::system_error(errno, std::system_category());

	// tolerable error
	setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &ONE, sizeof(ONE));

	int rc = bind(listenFD, &bindAddr.sockAddress, bindAddr.size());
	if (rc < 0)
		throw std::system_error(errno, std::system_category());

	rc = listen(listenFD, 128); //TODO: configurable backlog?
	if (rc < 0)
		throw std::system_error(errno, std::system_category());
}

void ListenReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;
	
	while (active)
	{
		int clientFD = accept4(listenFD, NULL, NULL, SOCK_NONBLOCK);
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
				throw std::system_error(errno, std::system_category());
			}
			continue;
		}
		
		const static int one = 1;
		setsockopt(clientFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int)); // tolerable error

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

void ListenReactor::start(bool defer)
{
	poller->add(this, listenFD, EPOLLIN);
}

ListenReactor::~ListenReactor()
{
	poller->remove(listenFD);
}
