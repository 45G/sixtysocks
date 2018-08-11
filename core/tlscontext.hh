#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread/tss.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	WOLFSSL_CTX *ctx;

	bool server;

	struct TicketCtx
	{
		byte keyName[WOLFSSL_TICKET_NAME_SZ];
		byte hmacKey[16];

		TicketCtx();
	};

	TicketCtx ticketCtx;

	struct Random
	{
		WC_RNG rng;

		Random();

		~Random();
	};

	boost::thread_specific_ptr<Random> random;

	static int ticketEncryptCallback(WOLFSSL* ssl, byte key_name[WOLFSSL_TICKET_NAME_SZ],
		byte iv[WOLFSSL_TICKET_IV_SZ], byte mac[WOLFSSL_TICKET_MAC_SZ],
		int enc, byte* ticket, int inLen, int* outLen,
		void* userCtx);

public:
	/* client context */
	TLSContext(const std::string &veriFile);

	/* server context */
	TLSContext(const std::string &certFile, const std::string keyFile);

	~TLSContext();
	
	WOLFSSL_CTX *get()
	{
		return ctx;
	}
	
	operator WOLFSSL_CTX *()
	{
		return ctx;
	}
	
	bool isServer() const
	{
		return server;
	}
	
	bool isClient() const
	{
		return !server;
	}
};

#endif // TLSCONTEXT_HH
