#include <stdexcept>
#include "tlsexception.hh"
#include "tlscontext.hh"
#include <boost/filesystem.hpp>
#include <wolfssl/test.h>

using namespace std;

/* client */
TLSContext::TLSContext(const std::string &veriFile)
	: server(false)
{
	ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
	if (ctx == NULL)
		throw runtime_error("Can't initialize WolfSSL context");
	
	try
	{
		int rc;
		
		boost::filesystem::path resolved = boost::filesystem::canonical(veriFile);
		if (boost::filesystem::is_directory(resolved))
			rc = wolfSSL_CTX_load_verify_locations(ctx, NULL, veriFile.c_str());
		else
			rc = wolfSSL_CTX_load_verify_locations(ctx, veriFile.c_str(), NULL);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
		
		rc = wolfSSL_CTX_UseSessionTicket(ctx);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
	}
	catch (std::exception &ex)
	{
		wolfSSL_CTX_free(ctx);
		throw ex;
	}
}

/* server */
TLSContext::TLSContext(const string &certFile, const string keyFile)
	: server(true)
{
	ctx = wolfSSL_CTX_new(wolfTLSv1_3_server_method());
	if (ctx == NULL)
		throw runtime_error("Can't initialize WolfSSL context");
	
	try
	{
		int rc;
		
		rc = wolfSSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
	
		rc = wolfSSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);

		wolfSSL_CTX_set_TicketEncCb(ctx, TLSContext::ticketEncryptCallback);
		wolfSSL_CTX_set_TicketEncCtx(ctx, this);
	}
	catch (std::exception &ex)
	{
		wolfSSL_CTX_free(ctx);
		throw ex;
	}
}

TLSContext::~TLSContext() {}

TLSContext::TicketCtx::TicketCtx()
{
	int rc = RAND_bytes(keyName, sizeof(keyName));
	if (rc < 0)
		throw runtime_error("Error doing RAND_bytes");

	rc = RAND_bytes(hmacKey, sizeof(hmacKey));
	if (rc < 0)
		throw runtime_error("Error doing RAND_bytes");
}

TLSContext::Random::Random()
{
	int rc = wc_InitRng(&rng);
	if (rc != 0)
		throw runtime_error("Error doing wc_InitRng");
}

TLSContext::Random::~Random()
{
	wc_FreeRng(&rng);
}

int TLSContext::ticketEncryptCallback(WOLFSSL *ssl, byte keyName[WOLFSSL_TICKET_NAME_SZ],
	byte iv[WOLFSSL_TICKET_IV_SZ], byte mac[WOLFSSL_TICKET_MAC_SZ],
	int enc, byte *ticket, int inLen, int *outLen, void *userCtx)
{
	(void)ssl; (void)ticket;

	int rc;

	unsigned hmacLen = WOLFSSL_TICKET_MAC_SZ;
	HMAC_CTX hCtx;

	TLSContext *tlsContext = reinterpret_cast<TLSContext *>(userCtx);

	if (enc)
	{
		memcpy(keyName, tlsContext->ticketCtx.keyName, WOLFSSL_TICKET_NAME_SZ);

		try
		{
			if (tlsContext->random.get() == NULL)
				tlsContext->random.reset(new Random());
		}
		catch (exception &)
		{
			return WOLFSSL_TICKET_RET_REJECT;
		}
		rc = wc_RNG_GenerateBlock(&tlsContext->random->rng, iv, WOLFSSL_TICKET_IV_SZ);
		if (rc != 0)
			return WOLFSSL_TICKET_RET_REJECT;

		*outLen = 0;

		HMAC_CTX_init(&hCtx);
		HMAC_Init_ex(&hCtx, tlsContext->ticketCtx.hmacKey, WOLFSSL_TICKET_MAC_SZ, EVP_sha256(), NULL);
		HMAC_Update(&hCtx, keyName, WOLFSSL_TICKET_NAME_SZ);
		HMAC_Update(&hCtx, iv, WOLFSSL_TICKET_IV_SZ);
		HMAC_Final(&hCtx, mac, &hmacLen);
		HMAC_CTX_cleanup(&hCtx);
	}
	else
	{
		byte compMac[WOLFSSL_TICKET_MAC_SZ];

		if (memcmp(tlsContext->ticketCtx.keyName, keyName, WOLFSSL_TICKET_NAME_SZ))
			return WOLFSSL_TICKET_RET_FATAL;

		HMAC_CTX_init(&hCtx);
		HMAC_Init_ex(&hCtx, tlsContext->ticketCtx.hmacKey, WOLFSSL_TICKET_MAC_SZ, EVP_sha256(), NULL);
		HMAC_Update(&hCtx, keyName, WOLFSSL_TICKET_NAME_SZ);
		HMAC_Update(&hCtx, iv, WOLFSSL_TICKET_IV_SZ);
		HMAC_Final(&hCtx, compMac, &hmacLen);
		HMAC_CTX_cleanup(&hCtx);

		if (memcmp(compMac, mac, WOLFSSL_TICKET_MAC_SZ))
			return WOLFSSL_TICKET_RET_FATAL;

		*outLen = inLen;
	}

	return WOLFSSL_TICKET_RET_OK;
}
