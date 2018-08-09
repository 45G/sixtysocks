#ifndef TLSEXCEPTION_HH
#define TLSEXCEPTION_HH

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <exception>

class TLSException: public std::exception
{
	int err;
public:
	TLSException(int err)
		: err(err) {}
	
	TLSException(WOLFSSL *tls, int result)
		: err(wolfSSL_get_error(tls, result)) {}

	const char *what() const throw();
};

#endif // TLSEXCEPTION_HH
