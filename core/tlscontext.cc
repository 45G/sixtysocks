#include <stdexcept>
#include "tlsexception.hh"
#include "tlscontext.hh"
#include <boost/filesystem.hpp>

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

	rc = RAND_bytes(aesKey, sizeof(aesKey));
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

int TLSContext::TicketCtx::encrypt(unsigned char *ticket, int size, unsigned char *iv)
{
	int ret = -1;
	int len;
	int outLen = 0;
	int rc;

	EVP_CIPHER_CTX *aesCtx = EVP_CIPHER_CTX_new();
	if (aesCtx == NULL)
		return -1;

	rc = EVP_EncryptInit_ex(aesCtx, EVP_aes_128_cbc(), NULL, aesKey, iv);
	if (rc != 1)
		goto done;

	rc = EVP_EncryptUpdate(aesCtx, ticket, &len, ticket, size);
	if (rc != 1)
		goto done;
	outLen += len;

	rc = EVP_EncryptFinal_ex(aesCtx, ticket + outLen, &len);
	if (rc != 1)
		goto done;
	outLen += len;

	ret = outLen;

done:
	EVP_CIPHER_CTX_free(aesCtx);
	return ret;
}

int TLSContext::TicketCtx::decrypt(unsigned char *ticket, int size, unsigned char *iv)
{
	int ret = -1;
	int len;
	int outLen = 0;
	int rc;

	EVP_CIPHER_CTX *aesCtx = EVP_CIPHER_CTX_new();
	if (aesCtx == NULL)
		return -1;

	rc = EVP_DecryptInit_ex(aesCtx, EVP_aes_128_cbc(), NULL, aesKey, iv);
	if (rc != 1)
		goto done;

	rc = EVP_DecryptUpdate(aesCtx, ticket, &len, ticket, size);
	if (rc != 1)
		goto done;
	outLen += len;

	rc = EVP_DecryptFinal_ex(aesCtx, ticket + outLen, &len);
	if (rc != 1)
		goto done;
	outLen += len;

	ret = outLen;

done:
	EVP_CIPHER_CTX_free(aesCtx);
	return ret;
}

int TLSContext::TicketCtx::hmac(const unsigned char *name, const unsigned char *iv, const unsigned char *ticket, int ticketLen, unsigned char *hash)
{
	HMAC_CTX hCtx;
	unsigned int hmacLen;
	int rc;
	int ret = -1;

	rc = HMAC_CTX_init(&hCtx);
	if (rc != 1)
		return -1;

	rc = HMAC_Init_ex(&hCtx, hmacKey, TLSContext::TicketCtx::HMAC_KEY_SIZE, EVP_sha256(), NULL);
	if (rc != 1)
		goto done;

	rc = HMAC_Update(&hCtx, name, WOLFSSL_TICKET_NAME_SZ);
	if (rc != 1)
		goto done;

	rc = HMAC_Update(&hCtx, iv, WOLFSSL_TICKET_IV_SZ);
	if (rc != 1)
		goto done;

	rc = HMAC_Update(&hCtx, ticket, ticketLen);
	if (rc != 1)
		goto done;

	rc = HMAC_Final(&hCtx, hash, &hmacLen);
	if (rc != 1)
		goto done;

	ret = hmacLen;

done:
	HMAC_CTX_cleanup(&hCtx);
	return ret;
}

int TLSContext::ticketEncryptCallback(WOLFSSL *ssl, byte keyName[WOLFSSL_TICKET_NAME_SZ],
	byte iv[WOLFSSL_TICKET_IV_SZ], byte mac[WOLFSSL_TICKET_MAC_SZ],
	int enc, byte *ticket, int inLen, int *outLen, void *userCtx)
{
	(void)ssl; (void)ticket;

	int rc;

	TLSContext *tlsContext = reinterpret_cast<TLSContext *>(userCtx);

	if (enc)
	{
		/* name */
		memcpy(keyName, tlsContext->ticketCtx.keyName, WOLFSSL_TICKET_NAME_SZ);

		/* iv */
		try
		{
			if (tlsContext->random.get() == NULL)
				tlsContext->random.reset(new Random());
		}
		catch (exception &)
		{
			return WOLFSSL_TICKET_RET_FATAL;
		}
		rc = wc_RNG_GenerateBlock(&tlsContext->random->rng, iv, WOLFSSL_TICKET_IV_SZ);
		if (rc != 0)
			return WOLFSSL_TICKET_RET_FATAL;

		/* ticket */
		*outLen = tlsContext->ticketCtx.encrypt(ticket, inLen, iv);
		if (*outLen == -1)
			return WOLFSSL_TICKET_RET_FATAL;

		/* hmac */
		rc = tlsContext->ticketCtx.hmac(keyName, iv, ticket, *outLen, mac);
		if (rc == -1)
			return WOLFSSL_TICKET_RET_FATAL;
	}
	else
	{
		/* name */
		if (memcmp(tlsContext->ticketCtx.keyName, keyName, WOLFSSL_TICKET_NAME_SZ))
			return WOLFSSL_TICKET_RET_REJECT;

		/* hmac */
		byte compMac[WOLFSSL_TICKET_MAC_SZ];
		rc = tlsContext->ticketCtx.hmac(keyName, iv, ticket, inLen, compMac);
		if (rc == -1)
			return WOLFSSL_TICKET_RET_FATAL;
		if (memcmp(compMac, mac, WOLFSSL_TICKET_MAC_SZ))
			return WOLFSSL_TICKET_RET_REJECT;

		/* ticket */
		*outLen = tlsContext->ticketCtx.decrypt(ticket, inLen, iv);
		if (*outLen == -1)
			return WOLFSSL_TICKET_RET_FATAL;
	}

	return WOLFSSL_TICKET_RET_OK;
}
