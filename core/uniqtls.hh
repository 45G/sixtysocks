#ifndef UNIQTLS_HH
#define UNIQTLS_HH

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class UniqTLS
{
	WOLFSSL *tls;
	
public:
	UniqTLS()
		: tls(NULL) {}
	
	void assign(WOLFSSL *tls)
	{
		assert(this->tls == NULL);
		this->tls = tls;
	}
	
	operator WOLFSSL *() const
	{
		return tls;
	}
	
	WOLFSSL *dupWrite()
	{
		WOLFSSL *ret = wolfSSL_write_dup(tls);
		return ret;
	}
	
	WOLFSSL *dupRead()
	{
		WOLFSSL *writeTLS = wolfSSL_write_dup(tls);
		if (writeTLS == NULL)
			return NULL;
		WOLFSSL *readTLS = tls;
		tls = writeTLS;
		return readTLS;
	}
	
	void reset()
	{
		if (tls != NULL)
		{
			wolfSSL_free(tls);
			tls = NULL;
		}
	}
	
	~UniqTLS()
	{
		if (tls != NULL)
			wolfSSL_free(tls);
	}
}

#endif // UNIQTLS_HH
