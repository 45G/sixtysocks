#ifndef TLSSESSION_HH
#define TLSSESSION_HH

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSSession
{
	WOLFSSL_SESSION *session;
	
public:
	TLSSession()
		: session(NULL) {}
	
	WOLFSSL_SESSION *get()
	{
		return session;
	}
	
	void set(WOLFSSL_SESSION *session)
	{
		this->session = session;
	}
};

#endif // TLSSESSION_HH
