#ifndef SSLEXCEPTION_HH
#define SSLEXCEPTION_HH

#include <exception>

class SSLException: public std::exception
{
	unsigned long err;
public:
	SSLException(unsigned long err)
		: err(err) {}

	const char *what() const throw();
};

#endif // SSLEXCEPTION_HH
