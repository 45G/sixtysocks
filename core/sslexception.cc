#include <openssl/err.h>
#include "sslexception.hh"

const char *SSLException::what() const throw()
{
	return ERR_reason_error_string(err);
}
