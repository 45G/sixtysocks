#ifndef TLSEXCEPTION_HH
#define TLSEXCEPTION_HH

#include <exception>
#include <nspr.h>

class TLSException: public std::exception
{
	PRErrorCode err;
public:
	TLSException(PRErrorCode err)
		: err(err) {}
	
	TLSException()
		: err(PR_GetError()) {}

	const char *what() const throw();
};

#endif // TLSEXCEPTION_HH
