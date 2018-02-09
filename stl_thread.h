/* ---------------------------------------------------------------------*
*                    Advanced Network Services v.o.f.,                  *
*              Copyright (c) 2002-2011 All Rights reserved              *
*                                                                       *
* This Library is licensed under the GNU general public license (GPL).  *
* As such, all software using it must also be released as open source   *
* under the same license. Read more about the GPL at www.gnu.org.       *
*                                                                       *
* With the alternative license agreement presented below it will        *
* be possible to use the library in your closed-source application.     *
*                                                                       *
* This file and its contents are protected by Dutch and                 *
* International copyright laws.  Unauthorized reproduction and/or       *
* distribution of all or any portion of the code contained herein       *
* is strictly prohibited and will result in severe civil and criminal   *
* penalties.  Any violations of this copyright will be prosecuted       *
* to the fullest extent possible under law.                             *
*                                                                       *
* THE SOURCE CODE CONTAINED HEREIN AND IN RELATED FILES IS PROVIDED     *
* TO THE REGISTERED DEVELOPER FOR THE PURPOSES OF EDUCATION AND         *
* TROUBLESHOOTING. UNDER NO CIRCUMSTANCES MAY ANY PORTION OF THE SOURCE *
* CODE BE DISTRIBUTED, DISCLOSED OR OTHERWISE MADE AVAILABLE TO ANY     *
* THIRD PARTY WITHOUT THE EXPRESS WRITTEN CONSENT OF THE OWNER          *
*                                                                       *
* THE REGISTERED DEVELOPER ACKNOWLEDGES THAT THIS SOURCE CODE           *
* CONTAINS VALUABLE AND PROPRIETARY TRADE SECRETS OF                    *
* ADVANCED NETWORK SERVICES THE REGISTERED DEVELOPER AGREES TO          *
* EXPEND EVERY EFFORT TO INSURE ITS CONFIDENTIALITY.                    *
*                                                                       *
* THE END USER LICENSE AGREEMENT (EULA) ACCOMPANYING THE PRODUCT        *
* PERMITS THE REGISTERED DEVELOPER TO REDISTRIBUTE THE PRODUCT IN       *
* EXECUTABLE FORM ONLY IN SUPPORT OF APPLICATIONS WRITTEN USING         *
* THE PRODUCT.  IT DOES NOT PROVIDE ANY RIGHTS REGARDING THE            *
* SOURCE CODE CONTAINED HEREIN.                                         *
*                                                                       *
* THIS COPYRIGHT NOTICE MAY NOT BE REMOVED FROM THIS FILE.              *
* --------------------------------------------------------------------- *
*/
#ifndef _STL_THREAD_H_
#define _STL_THREAD_H_ 1

#if defined(_WIN32)
#  include <windows.h>
#endif
#if defined(__linux__)
#  include <pthread.h>
#  ifndef __int64
#    define __int64 long long
#  endif
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* default the threshold = 3000 = 3ms
*/
extern int usEventWarnThreshold;

/* Get the pid of current thread use this in stead of getpid();
 *	Return pid of current thread
 */
int stlGetThreadId(void);

/* enable or disable core dump files on Linux
** default this option is enabled
*/
void stlThreadCoredumpOnOff(int iEnable);

/* Return thread name of PID will be invalid after thread exit */
const char *stlThreadPidName(int iPid);

/* Return current thread name */
const char *stlThreadName(void);

/* set name of thread with specific proces and data pointer */
int stlThreadPidNameSet(void *pFx,void *pData, char *sName);
int stlThreadNameSet(const char *sName);

/* Do a forced thread switch */
void stlThreadSwitch(void);

/* Suspend execution for a number of milliseconds */
void stlMsWait(int iMsCnt);

/* Suspend execution for a number of seconds */
void stlSecWait(int iSeconds);

/* Timer with micro second precision */
unsigned int stlUsTimer(unsigned int uOld);

/* Timer with millisecond precision */
unsigned int stlMsTimer(unsigned int uOld);

/* timer with CPU cycle precision (faster than ms and us timer but depended on cpu clock speed) */
unsigned __int64 stlCpuTimer64(unsigned __int64 uOld);

/* aantal ms sinds 1-1-1970 linux epoch time */
long long stlMsTimerEpoch();


/* timer with CPU cycle precision (every 64k cycli is 1 increment)
** loops every 32 hours on 26 GHz CPU powered pc 
** on 2.6 GHz CPU it is about 37 ticks per mS or 37000 ticks per second
*/
unsigned int stlCpuTimer(unsigned int uOld);

/* return number of CPU cycles per ms
**    recalc  0  use saved value
**           >0  recalculate CPU per ms (takes about 200 ms)
*/
unsigned int stlCpuPerMs(int recalc);

/* Starts a new thread return pid
**   thFnc    pointer to thread start function
**   iPrio    Priority 0=Normal -20=Realtime 20=Idle
**   Param    optional data pointer to pass on to the thread
**   name     name of the thread
** Return new thread ID
*/
int stlThreadStart(void(*thFnc)(void *),int prio,void *Param,const char *name);

/* Sets thread priority (from -20 high to 20 low) 0=default
**   iPrio   new thread priority -20=realtime  20=idle 0=normal
*/
void stlThreadPriority(int iPrio);

/* Terminate current thread */
void stlThreadAbort(void);

/* Mutex structure definition */
#if defined(_WIN32)
struct stlMutex{
	HANDLE hHandle;
};
# define stlMutexInitializer {0}
#endif

#if defined (__linux__)
struct stlMutex{
	pthread_mutex_t mux;
};
# define stlMutexInitializer {PTHREAD_MUTEX_INITIALIZER}
#endif

typedef struct stlMutex stlMutex;

/* Lock this mutex */
void stlMutexLock(stlMutex *);

/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**       >0 (the mutex is already locked by another proces)
*/
int stlMutexTryLock(stlMutex *mux);

/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**        1 (timeout)
**       >1 (other internal error)
*/
int stlMutexLockTo(stlMutex *mux,int iTimeOutMs);

/* Unlock this mutex */
void stlMutexUnlock(stlMutex *);

/* Initialize a new unlocked mutex */
void stlMutexInit(stlMutex *);

/* Release resources for a mutex */
void stlMutexRelease(stlMutex *);

/* Create a event
** return <>0 Ok
**          0 Error
*/
int stlEventCreate(int bManualReset,int bInitialState);

/* Delete event
** return <>0 Ok
**          0 Error
*/
int stlEventDelete(int iHdl);

/* Activate (raise) event
** return <>0 Ok
**          0 Error
*/
int stlEventSet(int iHdl);

/* Deactivate (lower) event
** return <>0 Ok
**          0 Error
*/
int stlEventReset(int iHdl);

/* Wait til event activates
** return 1 Event signaled
**        2 Timeout 
**        0 Error
*/
int stlEventWaitTo(int iHdl,int iTimeOut);

/* Wait til event activates
** return 1 Event signaled
**        0 Error
*/
int stlEventWait  (int iHdl);

/* Activate all threads waiting for this event exactly 1 time
**  iHdl  Handle to event
*/
void stlEvent1Shot(int iHdl);
void stlEvent1ShotFast(int iHdl);	// idem but don't wait for all threads to fire up.


typedef void  (*stlTimeEvent)(void *);

/* Laat een bepaald functie met bepaalde interval uitvoeren.
** Deze interval is exact en niet onderhevig aan systeem belasting max jitter 2ms.
**  timeFx     functie welke met interval uitgevoerd moet worden.
**  msInterval interval in milliseconden waarin deze functie aangeroepen moet worden.
**  pUserData  user defined data word aan functie weer meegegeven.
** Return 0 Ok
**       -1 out of memory
*/
int stlTimeEventAdd(stlTimeEvent timeFx,int iMsInterval,void *pUserData);

/* Laat een bepaald functie met bepaalde interval uitvoeren.
** Deze interval is exact en niet onderhevig aan systeem belasting max jitter 2ms.
**  timeFx     functie welke met interval uitgevoerd moet worden.
**  msInterval interval in milliseconden waarin deze functie aangeroepen moet worden.
**  pUserData  user defined data word aan functie weer meegegeven.
**  sName      name of the thread during execution (for debug simplification)
** Return 0 Ok
**       -1 out of memory
*/
int stlTimeEventAddN(stlTimeEvent timeFx,int iMsInterval,void *pUserData,const char *sName);

/* adjust / update name of ms event functie
*/
int stlTimeEventNameSet(stlTimeEvent timeFx,void *pUserData,const char *sName);

/* adjust interval tijd van een ms event functie
*/
int stlTimeEventIntervalSet(stlTimeEvent timeFx,void *pUserData,int iMsInterval);

/* Abort al time events (call before exiting program)
*/
void stlTimeEventsExit();

void stlTimeEventReport();	// only if enabled


/* Verwijder een event uit de timer event list
**  timeFx     pointer naar event functie
**  pUserData  als deze NULL is worden alle verwijzingen naar deze functie verwijderd.
**             indien deze een waarde bevat word alleen de functie met deze waarde verwijderd
** Return 0 Ok
*/
int stlTimeEventDel(stlTimeEvent timeFx,void *pUserData);


/* Set integer to specific value after specified time
**   pData    pointer to set value in
**   pValue   Value to write to data after delay.
**   iValLen  number of bytes in pValue
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -1 to many bytes to write
**       -2 no more room in database
*/
int stlTimeSetData(void *pData,void *pValue,int iValLen,int iMsDelay);
int stlTimeSetPtr(void **pDst,void *pSet,int iMsDelay);

/* Set integer to specific value after specified time
**   pDstVal  pointer to integer to set value in
**   setVal   Value to write to integer.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetInt(int *pDstVal,int setVal,int iMsDelay);
int stlTimeSetIntBit(int *pDstVal,int setBit,int iMsDelay);
int stlTimeClrIntBit(int *pDstVal,int clrBit,int iMsDelay);
int stlTimeSetClrDataMask32(int *pData,int bitMask,int iMsDelay,int iSetClr);
int stlTimeSetClrDataMask16(unsigned short *pData,unsigned short bitMask,int iMsDelay,int iSetClr);

/* Set byte to specific value after specified time
**   pDstVal  pointer to byte to set value in
**   setVal   Value to write to byte.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetByte(char *pDstVal,char setVal,int iMsDelay);

/* Set float to specific value after specified time
**   pDstVal  pointer to float to set value in
**   setVal   Value to write to float.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetFloat(float *pDstVal,float setVal,int iMsDelay);

/* Set double to specific value after specified time
**   pDstVal  pointer to double to set value in
**   setVal   Value to write to double.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetDouble(double *pDstVal,double setVal,int iMsDelay);

#ifdef  __cplusplus
}

/*
class stlCThread
{
private:
	int threadPrio;
	bool enabled;
public:
	// bool enableThread;
	// prio thread priority (from -20 high to 20 low/idle 0=default)
	stlCThread();
	stlCThread(int prio);
	stlCThread(char *name);
	stlCThread(int prio,char *name);
	virtual ~stlCThread(){};
public:
	// worker function you can do wat you want here
	// this function is only called once.
	virtual void worker() = 0;
	int threadName(char *fmt,...);		// adjust name of this proces
};

class stlCmsEvent
{
private:
	int evInterval;
	bool enabled;
public:
	//bool enableEvent;
	stlCmsEvent();
	stlCmsEvent(int msInterval);
	stlCmsEvent(int msInterval,char *myName);
	virtual ~stlCmsEvent(){};
public:
	// event handler you may not wait or take long computations in this event.
	// the timer is exactly in interval of ms.
	// so event on 1 ms is exactly 1000 events every second. (jitter max 2.5 ms on multi core systems)
	virtual void msEvent() = 0;
	int eventName(char *fmt,...);		// adjust name of this event
	int eventInterval(int msInterval);	// adjust interval of this event
};
*/

/************************************************************************

class myTest: stlCThread,stlCmsEvent
{
public:
	myTest():stlCThread(-20),stlCmsEvent(500){
		threadName("worker %d",5);
		eventName("event %d",6);
		enableEvent = enableThread = true;
	}
	virtual void worker()
	{
		while (enableThread)
		{
			stlMsWait(500);
			printf("worker thread %s",stlThreadName());
		}
	}
	virtual void msEvent()
	{
		printf("event handle %s",stlThreadName());
	}
};

***********************************************************************/



#endif

#endif

