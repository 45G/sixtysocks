#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread/tss.hpp>
#include <ssl.h>
#include <boost/intrusive_ptr.hpp>
#include "tlslibrary.hh"

class TLSContext: public boost::intrusive_ref_counter<TLSContext>
{
	bool server;

public:
	TLSContext(bool server);

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
