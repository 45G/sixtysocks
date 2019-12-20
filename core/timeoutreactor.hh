#ifndef TIMEOUTREACTOR_HH
#define TIMEOUTREACTOR_HH

#include <unordered_map>
#include <boost/intrusive/list.hpp>
#include <tbb/spin_mutex.h>
#include "reactor.hh"
#include "uniqfd.hh"
#include "timer.hh"

class TimeoutReactor: public Reactor
{
	static constexpr timespec INTERVAL = {
		.tv_sec  = 1,
		.tv_nsec = 0,
	};

	UniqFD fd;

	std::unordered_map<int, std::pair<boost::intrusive::list<Timer>, tbb::spin_mutex>> timers;

public:
	TimeoutReactor(Poller *poller, std::vector<int> possibleTimeouts);

	~TimeoutReactor();

	void add(Timer *timer, std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock().now())
	{
		assert(timers.find(timer->interval) != timers.end());
		assert(timer->tracker == nullptr);

		timer->tracker = this;
		timer->arm = now;

		auto &[queue, lock] = timers[timer->interval];
		tbb::spin_mutex::scoped_lock scopedLock(lock);
		queue.push_back(*timer);
	}

	void refresh(Timer *timer, std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock().now())
	{
		assert(timers.find(timer->interval) != timers.end());
		assert(timer->tracker == this);

		timer->arm = now;

		auto &[queue, lock] = timers[timer->interval];
		tbb::spin_mutex::scoped_lock scopedLock(lock);
		if (!timer->is_linked())
			return;
		queue.erase(queue.iterator_to(*timer));
		queue.push_back(*timer);
	}

	void cancel(Timer *timer)
	{
		assert(timers.find(timer->interval) != timers.end());
		assert(timer->tracker == this);

		auto &[queue, lock] = timers[timer->interval];
		tbb::spin_mutex::scoped_lock scopedLock(lock);
		if (!timer->is_linked())
			return;
		queue.erase(queue.iterator_to(*timer));
	}

	void start();

	void process(int fd, uint32_t events);

	void deactivate();
};

#endif // TIMEOUTREACTOR_HH
