#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread/tss.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	struct TicketCtx
	{
		static const int AES_KEY_SIZE  = 16;
		static const int HMAC_KEY_SIZE = 16;

		byte keyName[WOLFSSL_TICKET_NAME_SZ];
		byte aesKey[AES_KEY_SIZE];
		byte hmacKey[HMAC_KEY_SIZE];

		TicketCtx();

		int encrypt(unsigned char *ticket, int size, unsigned char *iv);

		int decrypt(unsigned char *ticket, int size, unsigned char *iv);

		int hmac(const unsigned char *name, const unsigned char *iv, const unsigned char *ticket, int ticketLen, unsigned char *hash);
	};

	WOLFSSL_CTX *ctx;

	bool server;

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
