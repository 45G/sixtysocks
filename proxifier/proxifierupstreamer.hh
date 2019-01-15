#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include <socks6util/socks6util.hh>
#include "../authentication/syncedtokenstuff.h"
#include "../core/streamreactor.hh"
#include "windowsupplicant.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_CONNECTING,
		S_HANDSHAKING,
		S_STREAM,
	};
	
	boost::intrusive_ptr<Proxifier> proxifier;
	
	State state;
	
	std::shared_ptr<SyncedTokenWallet> wallet;
	
	S6U::SocketAddress dest;
	
	std::shared_ptr<WindowSupplicant> windowSupplicant;
	
public:
	ProxifierUpstreamer(Proxifier *proxifier, int *pSrcFD, TLSContext *clientCtx, std::shared_ptr<WindowSupplicant> windowSupplicant);

	void start();
	
	void process(int fd, uint32_t events);
	
	Proxifier *getProxifier()
	{
		return proxifier.get();
	}
	
	std::shared_ptr<SyncedTokenWallet> getWallet() const
	{
		return wallet;
	}
	
	std::shared_ptr<WindowSupplicant> getSupplicant()
	{
		return windowSupplicant;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
