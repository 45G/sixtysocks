#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include <socks6util/socks6util.hh>
#include "../authentication/lockabletokenstuff.h"
#include "../core/streamreactor.hh"
#include "windowsupplicant.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_CONNECTING,
		S_STREAM,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state;
	
	boost::shared_ptr<LockableTokenWallet> wallet;
	
	S6U::SocketAddress dest;
	
	boost::shared_ptr<WindowSupplicant> supplicant;
	
public:
	ProxifierUpstreamer(Proxifier *proxifier, int srcFD, bool supplicate = false);
	
	void process(int fd, uint32_t events);
	
	Proxifier *getProxifier()
	{
		return proxifier.get();
	}
	
	boost::shared_ptr<LockableTokenWallet> getWallet() const
	{
		return wallet;
	}
	
	boost::shared_ptr<WindowSupplicant> getSupplicant()
	{
		return supplicant;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
