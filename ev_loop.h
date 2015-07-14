#ifndef __EV_LOOP_H_
#define __EV_LOOP_H_

#include "ev_timer.h"

namespace ev {
    
class loop {
	int ep_fd; // this is a socket used by epoll interface to report back events
	int max_epoll_events; // max number of epoll events to process at once
	epoll_event* epoll_events; // a memory area preallocated for reported epoll events
	
	// last error seen by any operation done by objects of this class
	string _last_error;
	
	list<timer> active_timers; // list of active timers
	
public:
	loop(void);
	~loop(void);
	
	const string& last_error(void) const { return _last_error; }
	
	// execute the event loop
	void exec(void);
	
	// add/remove active timers
	void add(timer);
	void remove(timer);
	
	// return epoll fd
	const int get_epfd(void) const { return ep_fd; }
	
	// initialize the event loop
	bool init(void);
    };
};

#endif // __EV_LOOP_H_
