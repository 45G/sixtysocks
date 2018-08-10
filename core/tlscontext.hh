#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	WOLFSSL_CTX *ctx;
	
	WOLFSSL_SESSION *session;

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

	void setSession(WOLFSSL_SESSION *session)
	{
		this->session = session;
	}

	WOLFSSL_SESSION *getSession()
	{
		return session;
	}
};

#endif // TLSCONTEXT_HH
