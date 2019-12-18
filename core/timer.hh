#ifndef TIMER_HH
#define TIMER_HH

#include <chrono>
#include <boost/intrusive/list.hpp>

class Reactor;
class TimeoutReactor;

class Timer: public boost::intrusive::list_base_hook<>
{
	TimeoutReactor *tracker = nullptr;

	std::chrono::time_point<std::chrono::system_clock> arm;
	int interval;

public:
	boost::intrusive::list_member_hook<> hook;

	Timer(int interval)
		: interval(interval) {}

	virtual void trigger() = 0;

	virtual ~Timer();

	void refresh();

	void cancel();

	friend class TimeoutReactor;
};

class ReactorInactivityTimer: public Timer
{
	Reactor *reactor;

public:
	ReactorInactivityTimer(int interval, Reactor *reactor)
		: Timer(interval), reactor(reactor) {}

	void trigger();
};

#endif // TIMER_HH
