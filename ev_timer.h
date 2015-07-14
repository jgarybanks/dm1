#ifndef __EV_TIMER_H_
#define __EV_TIMER_H_

#include "ev_timeval.h"
#include "mem_rc.h"

namespace ev {
    
class loop;
    
class timer_base : public sigc::trackable, public mem::obj<timer_base> {

	bool _is_running;
	bool _is_zero; // zero-timers are used to generate immediate evnets 
	timeval tv; // timeout
	timeval exp_on; // expires on
	loop* evl; // event loop

public:
	static long alive_nr;
	static long total_nr;

	timer_base(void);
	~timer_base(void);
	
	void attach(loop*);

	// start timer
	bool start(int sec, int usec = 0);

	// stop timer
	void stop(void);

	// restart using previously configured timeval
	bool restart(void);

	// emit alarm
	void emit_alarm(void);

	// is-zero timer is a timer which was started with 0 timeout
	// such timers are used to generate events during the very next event loop,
	// which is handy for error propagation without risking a high stack usage
	bool is_zero(void) const { return _is_zero; }

	//
	// signals
	//
	sigc::signal<void> alarm;
	
	//
	// accessory functions
	//
	bool is_expired(const struct timeval& curr_tv );
	const timeval& expires_on(void) const { return exp_on; }
	bool is_running(void) const { return _is_running; }
	ev::loop* get_loop(void) const { return evl; }

	string str(void) const { return "tv=" + tv.str() + " exp_on=" + exp_on.str() + " is_running=" + itos(_is_running); }
};

typedef mem::h0<timer_base> timer;

inline bool
timer_base::is_expired(const timeval& curr_tv)
{
	return _is_running && exp_on < curr_tv;
}
    
inline void
timer_base::attach(ev::loop* _evl)
{
	if (evl) {
		errlog << "trying to set evl twice\n";
		DIE();
	}
	evl = _evl;
}

};

#endif // __EV_TIMER_H_
