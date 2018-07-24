#include <stdlib.h>
#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

using namespace std;

void Proxy::handleNewConnection(int fd)
{
    ProxyUpstreamer *upstreamReactor = NULL;
    try
    {
        upstreamReactor = new ProxyUpstreamer(this, fd);
	}
	catch (...)
	{
		close(fd); // tolerable error
		return;
	}
	
    upstreamReactor->start(true);
}

LockableTokenBank *Proxy::createBank(const string &user, uint32_t size)
{
	ScopedSpinlock lock(&bankLock); (void)lock;
	LockableTokenBank *bank = new LockableTokenBank((uint32_t)rand(), size, 0, size / 2);
	
	banks[user] = unique_ptr<LockableTokenBank>(bank);
	return bank;
}

LockableTokenBank *Proxy::getBank(const string &user)
{
	ScopedSpinlock lock(&bankLock); (void)lock;
	
	return banks[user].get();
}
