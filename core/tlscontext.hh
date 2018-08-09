#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	class Ctx
	{
		WOLFSSL_CTX *ctx;
		
	public:
		Ctx()
			: ctx(NULL) {}
		~Ctx()
		{
			wolfSSL_CTX_free(ctx);
		}
		
		operator WOLFSSL_CTX *()
		{
			return ctx;
		}
		
		void operator =(WOLFSSL_CTX *otherCtx)
		{
			ctx = otherCtx;
		}
	};
	
	Ctx ctx;
	
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
