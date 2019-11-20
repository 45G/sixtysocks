#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <ssl.h>
#include <keyhi.h>
#include <pk11pub.h>
#include <sslexp.h>
#include "tlsexception.hh"
#include "tlslibrary.hh"

class TLSContext
{
	bool server;
	
	/* server stuff */
	std::string nick;
	std::unique_ptr<CERTCertificate,  void (*)(CERTCertificate  *)> cert { nullptr, CERT_DestroyCertificate };
	std::unique_ptr<SECKEYPrivateKey, void (*)(SECKEYPrivateKey *)> key  { nullptr, SECKEY_DestroyPrivateKey };
#ifndef SSL_SetupAntiReplay_NotMandatory
#ifdef SSL_CreateAntiReplayContext
	//TODO: make unique_ptr
	SSLAntiReplayContext *antiReplayCtx;
#endif
#endif
	
	/* client stuff */
	std::string sni;

public:
	TLSContext(bool server, const std::string &nick, const std::string &sni)
		: server(server), nick(nick), sni(sni)
	{
		if (server)
		{
			cert.reset(PK11_FindCertFromNickname(nick.c_str(), nullptr));
			if (!cert)
				throw std::runtime_error("Can't find certificate");
			
			key.reset(PK11_FindKeyByAnyCert(cert.get(), nullptr));
			if (!key)
				throw std::runtime_error("Can't find key");

			/* anti-replay */
			try
			{
#ifndef SSL_SetupAntiReplay_NotMandatory
#ifdef SSL_CreateAntiReplayContext
			static const int AR_WINDOW = 1;
			SECStatus status = SSL_CreateAntiReplayContext(PR_Now(), AR_WINDOW * PR_USEC_PER_SEC, 7, 14, &antiReplayCtx);
			if (status != SECSuccess)
				throw TLSException();
#endif
#endif
			}
			catch (TLSException &ex)
			{
				std::cerr << ex.what() << std::endl;
			}
		}
		else /* client */
		{
			if (sni == "")
				throw std::invalid_argument("SNI is required");
		}
	}

	~TLSContext()
	{
#ifndef SSL_SetupAntiReplay_NotMandatory
#ifdef SSL_CreateAntiReplayContext
		if (antiReplayCtx)
			SSL_ReleaseAntiReplayContext(antiReplayCtx); //might return error
#endif
#endif
	}
	
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

#ifndef SSL_SetupAntiReplay_NotMandatory
#ifdef SSL_CreateAntiReplayContext
	SSLAntiReplayContext *getAntiReplayCtx() const
	{
		return antiReplayCtx;
	}
#endif
#endif
};



#endif // TLSCONTEXT_HH
