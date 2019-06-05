#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <stdexcept>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread/tss.hpp>
#include <boost/intrusive_ptr.hpp>
#include <ssl.h>
#include <keyhi.h>
#include <pk11pub.h>
#include "tlslibrary.hh"

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	bool server;
	std::string nick;
	std::unique_ptr<CERTCertificate,  void (*)(CERTCertificate *)>  cert { nullptr, CERT_DestroyCertificate };
	std::unique_ptr<SECKEYPrivateKey, void (*)(SECKEYPrivateKey *)> key  { nullptr, SECKEY_DestroyPrivateKey };

public:
	TLSContext(bool server, const std::string &nick = "")
		: server(server), nick(nick)
	{
		if (server)
		{
			cert.reset(PK11_FindCertFromNickname(nick.c_str(), nullptr));
			if (cert.get() == nullptr)
				throw std::runtime_error("Can't find certificate");
			
			key.reset(PK11_FindKeyByAnyCert(cert.get(), nullptr));
			if (key.get() == nullptr)
				throw std::runtime_error("Can't find key");
		}
	}

	~TLSContext() {}
	
	bool isServer() const
	{
		return server;
	}
	
	bool isClient() const
	{
		return !server;
	}
	
	const std::string *getNick() const
	{
		return &nick;
	}
	
	CERTCertificate *getCert()
	{
		return cert.get();
	}
	
	SECKEYPrivateKey *getKey()
	{
		return key.get();
	}
};



#endif // TLSCONTEXT_HH
