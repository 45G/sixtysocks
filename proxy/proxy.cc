#include <stdlib.h>
#include <iostream>
#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

using namespace std;
using namespace tbb;
using namespace S6U;

const std::set<uint16_t> Proxy::DEFAULT_SERVICES = {
	53, /* DNS */
};

void Proxy::handleNewConnection(int fd)
{
	UniqFD ufd(fd);
	try
	{
		poller->assign(new ProxyUpstreamer(this, move(ufd)));
	}
	catch (exception &ex)
	{
		cerr << "Error handling new connection: " << ex.what() << endl;
	}
}

shared_ptr<ServerSession> Proxy::spawnSession()
{
	shared_ptr<ServerSession> ret;
	bool dupe;
	
	do
	{
		ret = make_shared<ServerSession>();
		concurrent_hash_map<uint64_t, shared_ptr<ServerSession>>::accessor ac;
		dupe = sessions.find(ac, ret->getID());
	}
	while (dupe);
	
	sessions.insert({ ret->getID(), ret });
	return ret;
}

std::shared_ptr<ServerSession> Proxy::getSession(uint64_t id)
{
	concurrent_hash_map<uint64_t, shared_ptr<ServerSession>>::accessor ac;
	bool found = sessions.find(ac, id);
	if (!found)
		return {};
	return ac->second;
}

SyncedTokenBank *Proxy::createBank(const string &user, uint32_t size)
{
	tbb::spin_mutex::scoped_lock lock(bankLock);
	SyncedTokenBank *bank = new SyncedTokenBank({ (uint32_t)rand(), size }, 0, size / 2);
	
	banks[user] = unique_ptr<SyncedTokenBank>(bank);
	return bank;
}

SyncedTokenBank *Proxy::getBank(const string &user)
{
	tbb::spin_mutex::scoped_lock lock(bankLock);
	
	return banks[user].get();
}
