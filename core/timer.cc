#include "timer.hh"
#include "timeoutreactor.hh"

Timer::~Timer()
{
	cancel();
}

void Timer::refresh()
{
	tracker->refresh(this);
}

void Timer::cancel()
{
	tracker->cancel(this);
}

void ReactorInactivityTimer::trigger()
{
	reactor->deactivate();
}
