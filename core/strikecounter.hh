#ifndef STRIKECOUNTER_HH
#define STRIKECOUNTER_HH

#include <stdexcept>

//TODO: do we really need this?
class StrikeCounter
{
	unsigned count = 0;
	unsigned max;
public:
	StrikeCounter(unsigned max)
		: max(max) {}

	void increment();

	class YoureOutException: public std::runtime_error
	{
	public:
		YoureOutException()
			: runtime_error("You're out!") {}
	};
};

#endif // STRIKECOUNTER_HH
