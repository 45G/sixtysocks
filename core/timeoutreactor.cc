#include "timeoutreactor.hh"

#include <sys/timerfd.h>
#include <stdexcept>
#include "poller.hh"

using namespace std;
using namespace std::chrono;
using namespace tbb;

TimeoutReactor::TimeoutReactor(Poller *poller, std::vector<int> possibleTimeouts)
	: Reactor(poller)
{
	fd.assign(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK));
	if (fd < 0)
		throw system_error(errno, system_category());

	for (int i: possibleTimeouts)
		timers[i];
}

TimeoutReactor::~TimeoutReactor()
{
	try
	{
		poller->remove(fd);
	}
	catch(...) {}
}

void TimeoutReactor::start()
{
	static constexpr itimerspec ITSPEC = {
		.it_interval = INTERVAL,
		.it_value    = INTERVAL,
	};

	int rc = timerfd_settime(fd, 0, &ITSPEC, nullptr);
	if (rc < 0)
		throw system_error(errno, system_category());

	poller->add(this, fd, Poller::IN_EVENTS);
}

void TimeoutReactor::process(int fd, uint32_t events)
{
	(void)fd; (void)events;

	auto now = system_clock().now();

	for (auto &entry: timers)
	{
		int timeout = entry.first;
		auto *queue = &(entry.second);

		spin_mutex::scoped_lock scopedLock(queue->second);

		while (!queue->first.empty())
		{
			Timer *timer = &(queue->first.front());
			if (timer->arm + milliseconds(timeout) < now)
				break;

			timer->trigger();
			queue->first.pop_front();
		}
	}
}

void TimeoutReactor::deactivate()
{
	Reactor::deactivate();
	poller->remove(fd);
}

