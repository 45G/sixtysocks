#include <stdlib.h>
#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

using namespace std;

void Proxy::handleNewConnection(int fd)
{
	try
	{
		poller->assign(new ProxyUpstreamer(this, &fd, serverCtx));
	}
	catch (exception &)
	{
		close(fd); // tolerable error
	}
}

SyncedTokenBank *Proxy::createBank(const string &user, uint32_t size)
{
	std::lock_guard<Spinlock> lock(bankLock);
	SyncedTokenBank *bank = new SyncedTokenBank((uint32_t)rand(), size, 0, size / 2);
	
	banks[user] = unique_ptr<SyncedTokenBank>(bank);
	return bank;
}

SyncedTokenBank *Proxy::getBank(const string &user)
{
	std::lock_guard<Spinlock> lock(bankLock);
	
	return banks[user].get();
}
