#include <stdlib.h>
#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

using namespace std;
using namespace tbb;

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

shared_ptr<ProxySession> Proxy::spawnSession()
{
	shared_ptr<ProxySession> ret;
	bool dupe;
	
	do
	{
		ret = make_shared<ProxySession>();
		concurrent_hash_map<uint64_t, shared_ptr<ProxySession>>::accessor ac;
		dupe = sessions.find(ac, ret->getId());
	}
	while (!dupe);
	
	sessions.insert({ ret->getId(), ret });
	return ret;
}

std::shared_ptr<ProxySession> Proxy::getSession(uint64_t id)
{
	concurrent_hash_map<uint64_t, shared_ptr<ProxySession>>::accessor ac;
	bool found = sessions.find(ac, id);
	if (!found)
		return {};
	return ac->second;
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
