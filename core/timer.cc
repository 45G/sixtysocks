#include "timer.hh"
#include "timeoutreactor.hh"

Timer::~Timer()
{
	tracker->cancel(this);
}

void ReactorInactivityTimer::trigger()
{
	reactor->deactivate();
}
