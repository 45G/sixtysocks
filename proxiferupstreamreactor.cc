#include "proxiferupstreamreactor.hh"

ProxiferUpstreamReactor::ProxiferUpstreamReactor(int srcFD)
	: StreamReactor(srcFD, -1), state(S_READING_INIT_DATA)
{
	
}

void ProxiferUpstreamReactor::process(Poller *poller, uint32_t events)
{
	if (events & EPOLLERR)
	{
		//TODO
	}
	
	if ((events & EPOLLIN) || (events & EPOLLOUT))
	{
		switch (state)
		{
		case S_READING_INIT_DATA:
		{
			sockaddr_storage dest;
			
			
			ssize_t bytes = recv(srcFD, buf.getHead(), buf.freeSize(), MSG_NOSIGNAL);
			if (bytes < 0)
			{
				if (errno != EWOULDBLOCK && errno != EAGAIN)
				{
					kill();
					return;
				}
			}
		}
		case S_WAITING_FOR_DATA:
		{
			break;
		}
		case S_CONNECTING:
		{
			break;
		}
		case S_SENDING_REQ:
		{
			break;
		}
		case S_WAITING_TO_SEND:
		{
			break;
		}
		}
	}
	
}

int ProxiferUpstreamReactor::getFD() const
{
	switch (state)
	{
	case S_READING_INIT_DATA:
	case S_WAITING_FOR_DATA:
		return srcFD;
	
	case S_CONNECTING:
	case S_SENDING_REQ:
	case S_WAITING_TO_SEND:
		return dstFD;
	}
}
