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
	std::unique_ptr<CERTCertificate,  void (*)(CERTCertificate  *)> cert { nullptr, CERT_DestroyCertificate };
	std::unique_ptr<SECKEYPrivateKey, void (*)(SECKEYPrivateKey *)> key  { nullptr, SECKEY_DestroyPrivateKey };
	
#ifdef SSL_CreateAntiReplayContext
	static void antiReplayCtxDeleter(SSLAntiReplayContext *antiReplayCtx);
	
	std::unique_ptr<SSLAntiReplayContext, void (*)(SSLAntiReplayContext *)> antiReplayCtx { nullptr, antiReplayCtxDeleter };
#endif
	
	/* client stuff */
	std::string sni;

public:
	TLSContext(bool server, const std::string &nick, const std::string &sni)
		: server(server), sni(sni)
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
#ifdef SSL_CreateAntiReplayContext
				static constexpr int AR_WINDOW = 1;
				SSLAntiReplayContext *ctx;
				SECStatus status = SSL_CreateAntiReplayContext(PR_Now(), AR_WINDOW * PR_USEC_PER_SEC, 7, 14, &ctx);
				antiReplayCtx.reset(ctx);
				if (status != SECSuccess)
					throw TLSException();
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
	
	bool isServer() const
	{
		return server;
	}
	
	bool isClient() const
	{
		return !server;
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

#ifdef SSL_CreateAntiReplayContext
	SSLAntiReplayContext *getAntiReplayCtx() const
	{
		return antiReplayCtx.get();
	}
#endif
};



#endif // TLSCONTEXT_HH
