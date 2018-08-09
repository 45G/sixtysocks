#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	WOLFSSL_CTX *ctx;
	
	bool server;
	
public:
	/* client context */
	TLSContext(const std::string &veriFile);

	/* server context */
	TLSContext(const std::string &certFile, const std::string keyFile);
	
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
