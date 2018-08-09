#ifndef TLSLIBRARY_HH
#define TLSLIBRARY_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSLibrary: public boost::intrusive_ref_counter<TLSLibrary>
{
public:
	TLSLibrary()
	{
		wolfSSL_Init();
	}
	
	~TLSLibrary()
	{
		wolfSSL_Cleanup();
	}
};

#endif // TLSLIBRARY_HH
