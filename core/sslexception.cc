#include <wolfssl/error-ssl.h>
#include "sslexception.hh"

const char *SSLException::what() const throw()
{
	return wc_GetErrorString(err);
}
