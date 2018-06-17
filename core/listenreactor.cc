#include <sys/socket.h>
#include <errno.h>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "poller.hh"
#include "listenreactor.hh"

using namespace std;

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
				throw system_error(errno, system_category());
			}
			continue;
		}
		
		const static int one = 1;
		setsockopt(clientFD, SOL_TCP, TCP_NODELAY, &one, sizeof(int)); // tolerable error

		handleNewConnection(clientFD);		
resched:
		poller->add(this, listenFD, EPOLLIN);
	}
}

void ListenReactor::deactivate()
{
	Reactor::deactivate();
	
	poller->remove(listenFD);
}

ListenReactor::~ListenReactor()
{
	poller->remove(listenFD);
	close(listenFD);
}
