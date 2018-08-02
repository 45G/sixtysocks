#ifndef TLSCONTEXT_HH
#define TLSCONTEXT_HH

#include <openssl/ssl.h>
#include <openssl/err.h>

class TLSContext
{
public:
	TLSContext();

	~TLSContext();
};

#endif // TLSCONTEXT_HH
