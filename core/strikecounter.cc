#include "strikecounter.hh"

const char *StrikeCounter::YoureOutException::what() const throw()
{
	return "You're out!";
}
