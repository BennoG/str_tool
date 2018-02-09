#ifndef _STL_HIRES_TIMER_H_
#define _STL_HIRES_TIMER_H_
#include "stl_str.h"

#ifdef __cplusplus

class stlHiresTimer
{
public:
	bool enable;
	stlHiresTimer();
	~stlHiresTimer();
	/* restart timer zodat tijd tussen stamp en start niet word mee genomen
	*/
	void start();
	/* Reset alle timers en start helemaal opnieuw met de metingen
	*/
	void reset();
	/* Mark een timestamp from start of vorige time stamp
	*/
	void stamp(const char *fmt,...);
	/* Report alle time stamps
	*/
	void report(const char *fmt = NULL,...);
	/* Get report in STP value
	*/
	STP reportMsg(const char *fmt = NULL,...);
	/* Get total number of us after reset
	*/
	int getUS();

private:
	int iTimeIx,iTimeA[20],iTimeS[20];
	STP sTimeD[20];
	void reportAddTimings(STP sMsg);
};

#endif	// __cplusplus

#endif	// _STL_HIRES_TIMER_H_

