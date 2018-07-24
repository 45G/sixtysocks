#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include <socks6util/socks6util.hh>
#include "../core/streamreactor.hh"

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
	
	bool supplicating;
	
	boost::shared_ptr<S6U::TokenWallet> wallet;
	
	S6U::SocketAddress dest;
	
public:
	ProxifierUpstreamer(Proxifier *proxifier, int srcFD, bool supplicate = false);
	
	~ProxifierUpstreamer();
	
	void process(int fd, uint32_t events);
	
	Proxifier *getProxifier()
	{
		return proxifier.get();
	}
	
	boost::shared_ptr<S6U::TokenWallet> getWallet() const
	{
		return wallet;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
