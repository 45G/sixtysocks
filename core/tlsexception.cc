#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/error-ssl.h>
#include "tlsexception.hh"

const char *TLSException::what() const throw()
{
	return wc_GetErrorString(err);
}
