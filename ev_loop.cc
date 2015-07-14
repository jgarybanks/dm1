
#include "ev_loop.h"
#include "ev_socket.h"
#include "ev_timer.h"

#include "misc.h"


namespace ev {
    
loop::loop(void) : ep_fd(-1)
{
}
    
loop::~loop(void)
{
	if (-1 != ep_fd)
	    ::close (ep_fd);
	
	delete[] epoll_events;
} 
    
bool
loop::init(void)
{
	if (-1 != ep_fd) {
		levc << "already initialized\n";
		return true;
	}
	
	ep_fd = epoll_create(1024);
	
	if (-1 == ep_fd) {
	    _last_error = strerror(errno);
	    errlog << "epoll_create(..) failed: " << last_error() << "\n";
	    return false;
	}
	
	max_epoll_events = 10*1024;
	epoll_events = new epoll_event[max_epoll_events];
	
	return true;
}

void
loop::exec(void)
{
	if (!init())
		return;
	
	while (true) {
		timeval curr_tv = timeval::current();
		levc << "curr_tv=" << curr_tv.str() << "\n";

		timeval next_tv; // invalid

		levc << active_timers.size() << " timer(s) in queue\n";

		// process timers
		int a_nr = 0;
		for (list<timer>::iterator it = active_timers.begin(); it != active_timers.end();) {
			a_nr++;

			if (!(*it)->is_running()) {
				levc << "timer is not running; erasing; info=" << (*it)->str();
				it = active_timers.erase(it);
				continue;
			}

			// process expired
			if ((*it)->is_expired(curr_tv)) {
				levc << "timer expired; info=" << (*it)->str() << "\n";
				timer t = *it;

				it = active_timers.erase(it);
				t->emit_alarm();

				if (t->is_running() && t->expires_on() < next_tv)
					next_tv = t->expires_on();
				continue;
			}

			if ((*it)->expires_on() < next_tv)
				next_tv = (*it)->expires_on();

			it++;
		}
		levc << a_nr << " timer(s)\n";

		usleep(50000);

		//
		// get the timeout to wait for, in microseconds
		//
		int timeout = -1;
		if (next_tv.is_valid()) {
			timeout = (next_tv - curr_tv).to_msec();
			timeout += 20; // 20 msec: adjustment for rounding errors
		}

		if (timeout < 0) {
			errlog << "!! got negative timeout=" << timeout << ", set to 50msec\n";
			timeout = 50;
		}

		levc << "use timeout=" << timeout << " msec\n";

		int rv = epoll_wait(ep_fd, epoll_events, max_epoll_events, timeout);

		if (-1 == rv && errno != EINTR) {
			errlog << "epoll_wait(..) ERROR: " << strerror(errno) << "\n";
			exit(-1);
		}

		// process socket events
		if (rv > 0) {
			levc << "epoll_wait: " << rv << " socket(s) to process\n";
			epoll_event* evl = epoll_events;
			while (rv--) {
				((ev::socket*)evl->data.ptr)->process(evl->events);
				evl++;
			}
		}
	}
}

void
loop::add(timer t)
{
	active_timers.push_back(t);
}

void
loop::remove(timer t)
{
	for (list<timer>::iterator it = active_timers.begin(); it != active_timers.end(); it++) {
		if (*it == t) {
			active_timers.erase(it);
			break;
		}
	}
}

};
