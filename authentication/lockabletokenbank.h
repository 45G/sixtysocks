#ifndef LOCKABLETOKENBANK_H
#define LOCKABLETOKENBANK_H

#include <socks6util/socks6util_idempotence.hh>
#include "../core/spinlock.hh"

class LockableTokenBank: public S6U::TokenBank
{
	Spinlock spinlock;
public:
	void acquire()
	{
		spinlock.acquire();
	}
	
	void attempt()
	{
		spinlock.attempt();
	}
	
	void release()
	{
		spinlock.release();
	}
}

#endif // LOCKABLETOKENBANK_H
