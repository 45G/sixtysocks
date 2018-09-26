#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread/tss.hpp>
#include <ssl.h>

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	//CERTCertDBHandle certDB;

	bool server;

public:
	/* client context */
	TLSContext(const std::string &veriFile);

	/* server context */
	TLSContext(const std::string &certFile, const std::string keyFile);

	~TLSContext();
	
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
