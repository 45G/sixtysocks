#ifndef STRIKECOUNTER_HH
#define STRIKECOUNTER_HH

#include <exception>

//TODO: do we really need this?
class StrikeCounter
{
	unsigned count;
	unsigned max;
public:
	StrikeCounter(unsigned max)
		: count(0), max(max) {}

	void increment();

	class YoureOutException: public std::exception
	{
	public:
		const char *what() const throw();
	};
};

#endif // STRIKECOUNTER_HH
