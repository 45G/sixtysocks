#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <stdexcept>
#include <boost/thread/tss.hpp>
#include <ssl.h>
#include <keyhi.h>
#include <pk11pub.h>
#include "tlslibrary.hh"

class TLSContext
{
	bool server;
	
	/* server stuff */
	std::string nick;
	std::unique_ptr<CERTCertificate,  void (*)(CERTCertificate  *)> cert { nullptr, CERT_DestroyCertificate };
	std::unique_ptr<SECKEYPrivateKey, void (*)(SECKEYPrivateKey *)> key  { nullptr, SECKEY_DestroyPrivateKey };
	
	/* client stuff */
	std::string sni;

public:
	TLSContext(bool server, const std::string &nick, const std::string &sni)
		: server(server), nick(nick), sni(sni)
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
		else /* client */
		{
			if (sni == "")
				throw std::invalid_argument("SNI is required");
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
	
	const std::string *getSNI() const
	{
		return &sni;
	}
};



#endif // TLSCONTEXT_HH
