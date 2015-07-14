
#include "ev_timer.h"
#include "ev_loop.h"
#include "misc.h"


namespace ev {
    
long timer_base::alive_nr = 0;
long timer_base::total_nr = 0;
    
    
timer_base::timer_base(void)
	: sigc::trackable(), mem::obj<timer_base>(),
	  _is_running(false),_is_zero(false),evl(NULL) {
	
	alive_nr++;
	total_nr++;
}

timer_base::~timer_base(void)
{
	stop();
	alive_nr--;
}

bool
timer_base::start(int sec, int usec)
{
	if (-1 == sec) {
		stop();
		return true;
	}

	bool was_running = _is_running;

	_is_zero = !sec && !usec;

	// calculate the expiration
	tv = timeval(sec,usec);
	timeval ctv = timeval::current();
	//exp_on = timeval::current() + tv;
	exp_on = ctv + tv;
	_is_running = true;
	
	if (!was_running)
	    evl->add(self());

	return true;
}

void
timer_base::stop (void)
{
	if (!_is_running)
	    return;

	//NOTE: event loop will ignore/remove timers which are not is_running()

	_is_running = false;
}

bool
timer_base::restart(void)
{
	if (tv.is_valid())
		return start(tv.tv().tv_sec, tv.tv().tv_usec);

	return true;
}

void
timer_base::emit_alarm(void)
{
	_is_running = false;
	alarm.emit();
}

};
