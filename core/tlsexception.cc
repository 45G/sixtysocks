#include "tlsexception.hh"

const char *TLSException::what() const throw()
{
	const char *str = PR_ErrorToName(err);
	if (str == nullptr)
		str = "Unknown error";
	return str;
}
