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
#define _WIN32_WINNT 0x0400
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#  include <process.h>
#endif
#ifdef __linux__
#  include <sys/resource.h>
#  include <sys/time.h>
#  include <unistd.h>
#  include <pthread.h>
#  include <errno.h>
#endif

#include "internal.h"
#include "stl_thread.h"

#define _MaxThread_	2048
#define _FlgInUse_	0x0001

static stlMutex mtxThTbl=stlMutexInitializer;

static struct threadIdData{
	int iFlgs;
	volatile int iPid;
	int iPrio;
	int iIdx;		// Index number in this array
	char sName[50];	// Name of the thread
	void *pData;	// Pointer to user data
#if defined (_WIN32)
	void (__cdecl *start_address)(void*);	// Thread definition function
#endif
#if defined (__linux__)
	void (*start_address)(void*);			// Thread definition function
#endif
}tIDT[_MaxThread_];

void stlThreadCoredumpOnOff(int iEnable)
{
#ifdef __linux__
	struct rlimit rlim;
	getrlimit(RLIMIT_CORE,&rlim);
	if (iEnable)
	{
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
	}else
	{
		rlim.rlim_cur = 0;
		rlim.rlim_max = 0;
	}
	setrlimit(RLIMIT_CORE,&rlim);
#endif
}


/* INTERNAL check if data needs initializing */
static void _stlThreadInit(void)
{
	static int iNeedInit=1;
	if (iNeedInit){
#ifdef __linux__
		struct rlimit rlim;
		getrlimit(RLIMIT_CORE,&rlim);
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE,&rlim);
#endif
		memset(tIDT,0,sizeof(tIDT));
		iNeedInit=0;
	}
}
static struct threadIdData *_stlNewThIdData(void)
{
	int i;
	_stlThreadInit();
	stlMutexLock(&mtxThTbl);
	for (i=0;i<_MaxThread_;i++){
		if (tIDT[i].iFlgs) continue;
		memset(&tIDT[i],0,sizeof(tIDT[i]));
		tIDT[i].iFlgs=_FlgInUse_;
		stlMutexUnlock(&mtxThTbl);
		return &tIDT[i];
	}
	stlMutexUnlock(&mtxThTbl);
	return NULL;
}

/* Get the pid of current thread use this in stead of getpid();
 *	Return pid of current thread
 */
int stlGetThreadId(void)
{
#ifdef __linux__
	return pthread_self();
	//return getpid();
#endif
#ifdef _WIN32
	return GetCurrentThreadId();
#endif
}

/* Return thread name of PID will be invalid after thread exit */
const char *stlThreadPidName(int iPid)
{
	int i;
	_stlThreadInit();
	for (i=0;i<_MaxThread_;i++){
		if (tIDT[i].iPid==iPid) return tIDT[i].sName;
	}
	return NULL;
}

/* set name of thread with specific ID */
int stlThreadPidNameSet(void *pFx,void *pData, char *sName)
{
	int i;
	if ((sName == NULL) || (sName[0]==0)) return -1;
	_stlThreadInit();
	for (i=0;i<_MaxThread_;i++){
		if ((tIDT[i].start_address == pFx  ) &&
			(tIDT[i].pData         == pData) )
		{
			strncpy(tIDT[i].sName, sName, sizeof(tIDT[0].sName)-1); 
			return 0;
		}
	}
	return -2;
}

/* sets the name of current thread */
int stlThreadNameSet(const char *sName)
{
	char *sOld = (char*) stlThreadPidName(stlGetThreadId());
	if ((sOld == NULL) || (sName == NULL) || (sName[0] == 0)) return -1;
	strncpy(sOld, sName, sizeof(tIDT[0].sName)-1); 
	return 0;
}

/* Return current thread name */
const char *stlThreadName(void)
{
	static char nullName[sizeof(tIDT[0].sName)]="main";
	const char *nam;
	int iPid;
	iPid=stlGetThreadId();
	nam=stlThreadPidName(iPid);
	if (nam==NULL) nam=nullName;
	return nam;
}

#if defined(_WIN32)
static struct threadIdData *_stlGetTD(void)
{
	int i,iPid=stlGetThreadId();
	for (i=0;i<_MaxThread_;i++)
		if (tIDT[i].iPid==iPid) 
			return &tIDT[i];
	return NULL;
}
#endif



void stlSecWait(int iSeconds)
{
	int i;
	for (i=0;i<iSeconds;i++) stlMsWait(1000);
}


#if defined(__linux__)
void stlThreadSwitch(void)
{
	usleep(0);
}
void stlMsWait(int iMsCnt)
{
	while (iMsCnt>1000){usleep(1000*1000);iMsCnt-=1000;}
	usleep(iMsCnt*1000);
}
/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**       >0 (the mutex is already locked by another proces)
*/
int stlMutexTryLock(stlMutex *mux)
{
	if (pthread_mutex_trylock(&mux->mux)==EBUSY) return 1;
	return 0;
}
/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**        1 (timeout)
**       >1 (other internal error)
*/
int stlMutexLockTo(stlMutex *mux,int iTimeOutMs)
{
    while (1)
    {
	int res = stlMutexTryLock(mux);
        if ( res == 0) return 0;	// we have the lock
	if (iTimeOutMs-- <= 0) return 1;
	stlMsWait(1);
    }
}

void stlMutexLock(stlMutex *mux)
{
	pthread_mutex_lock(&mux->mux);
}
void stlMutexUnlock(stlMutex *mux)
{
	pthread_mutex_unlock(&mux->mux);
}
void stlMutexInit(stlMutex *mux)
{
	pthread_mutex_init(&mux->mux,NULL);
}
void stlMutexRelease(stlMutex *mux)
{
	pthread_mutex_destroy(&mux->mux);
	memset(mux,0,sizeof(stlMutex));
}
static void *StartTask(void *p)
{
	struct threadIdData *TD=p;
	pthread_detach(pthread_self());
	TD->iPid=stlGetThreadId();
	//printf("stlThreadStart %d (%s)",(((int)pthread_self())>>12) & 0xFFFFF,TD->sName);
	TD->start_address(TD->pData);
	//printf("stlThreadStop %d (%s)",(((int)pthread_self())>>12) & 0xFFFFF,TD->sName);
	memset(TD,0,sizeof(struct threadIdData));
	pthread_exit(NULL);
}
void stlThreadAbort(void)
{
	int i,iPid=stlGetThreadId();
	for (i=0;i<_MaxThread_;i++){
		if (tIDT[i].iPid==iPid){
			memset(&tIDT[i],0,sizeof(struct threadIdData));
			break;
		}
	}
	pthread_exit(NULL);
}
// Sets thread priority (from -20 high to 20 low)
void stlThreadPriority(int iPrio)
{
	// values between -20(highest prio) and 20(lowest prio)
	setpriority(PRIO_PROCESS,0,iPrio);
}



/************************************************************************/
/* Event afhandeling                                                    */
/************************************************************************/

/*  verplaatst naar stl_thread_p.cpp 

#define _EvtMagicNumber_	0x456A42BA
#define _EvtFlgOneShot_		0x0001	// Geeft aan dat eer een one shot aan de gang is
#define _EvtFlgManReset_	0x0002	// als event manual reset is.
#define _EvtFlgSignaled_	0x0004	// als event gesignalled is

struct eventStruct{
	pthread_mutex_t mux,mWait;
	int iFlags;		// status flags
	int iMagic;
};

// Creëer een event
// return <>0 Ok
//          0 Error
int stlEventCreate(int bManualReset,int bInitialState)
{
	struct eventStruct *evt;
	evt=malloc(sizeof(struct eventStruct));
	if (evt==NULL) return 0;
	memset(evt,0,sizeof(struct eventStruct));
	pthread_mutex_init(&evt->mux  ,NULL);
	pthread_mutex_init(&evt->mWait,NULL);
	if (bManualReset ) evt->iFlags |= _EvtFlgManReset_;
	if (bInitialState) evt->iFlags |= _EvtFlgSignaled_;
	else               pthread_mutex_lock(&evt->mWait);
	evt->iMagic = _EvtMagicNumber_;
	return (int)evt;
}
// Delete event
// return <>0 Ok
//          0 Error
int stlEventDelete(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		pthread_mutex_destroy(&evt->mux  );
		pthread_mutex_destroy(&evt->mWait);
		memset(evt,0,sizeof(struct eventStruct));
		free(evt);
		return 1;
	}
	return 0;
}
// Activate event
// return <>0 Ok
//          0 Error
int stlEventSet(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		pthread_mutex_lock(&evt->mux);
		if ((evt->iFlags & _EvtFlgManReset_) && ((evt->iFlags & _EvtFlgSignaled_)==0))
		{	// these housekeeping are only needed when manual reset events are used
			evt->iFlags |= _EvtFlgSignaled_;
		}
		pthread_mutex_unlock(&evt->mWait);
		pthread_mutex_unlock(&evt->mux);
		return 1;
	}
	return 0;
}
// Deactivate event
// return <>0 Ok
//          0 Error
int stlEventReset(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		pthread_mutex_lock(&evt->mux);
		if ((evt->iFlags & _EvtFlgManReset_) && (evt->iFlags & _EvtFlgSignaled_))
		{
			evt->iFlags ^= _EvtFlgSignaled_;
			pthread_mutex_lock(&evt->mWait);
		}
		pthread_mutex_unlock(&evt->mux);
		return 1;
	}
	return 0;
}


// Wacht tot een event actief word
// return <>0 Ok
//          0 Error
int stlEventWait(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		pthread_mutex_lock(&evt->mWait);
		if (evt->iFlags & _EvtFlgOneShot_)
		{
			if (evt->iFlags & _EvtFlgManReset_)
			{	// Manual reset so other threads may run also
				pthread_mutex_unlock(&evt->mWait);	// other threads allowed
				pthread_mutex_lock(&evt->mux);
				pthread_mutex_unlock(&evt->mux);
				return 1;
			}
			pthread_mutex_lock(&evt->mux);
			pthread_mutex_unlock(&evt->mux);
		}
		if (evt->iFlags & _EvtFlgManReset_)
		{	// Manual reset so other threads may run also
			pthread_mutex_unlock(&evt->mWait);	// other threads allowed
		}
		return 1;
	}
	return 0;
}


// Activate all threads waiting for this event exactly 1 time
//  iHdl  Handle to event
void stlEvent1Shot(int iHdl)
{
	int i;
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		evt->iFlags |= _EvtFlgOneShot_;
		pthread_mutex_lock(&evt->mux);			// deze voorkomt dat een wachter meerdere rondjes draait tijdens de set event
		pthread_mutex_unlock(&evt->mWait);
		stlThreadSwitch();
		pthread_mutex_lock(&evt->mWait);
		evt->iFlags &= ~_EvtFlgOneShot_;
		pthread_mutex_unlock(&evt->mux);
		for (i=0;i<3;i++) stlThreadSwitch();	// Dit moeten er 100 zijn anders mis je events als er 50 threads op dit event 
	}
}


#ifdef __GLIBC_PREREQ
#if __GLIBC_PREREQ(2,4)
int stlEventWaitTo(int iHdl,int iTimeOut)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		struct timeval tv;
		struct timespec tmo;
		gettimeofday(&tv, NULL);
		memset(&tmo,0,sizeof(tmo));
		tmo.tv_sec  = tv.tv_sec + (iTimeOut / 1000);
		tmo.tv_nsec = tv.tv_usec * 1000;
		tmo.tv_nsec+=(iTimeOut % 1000) * 1000000;
		while (tmo.tv_nsec > 1000000000){tmo.tv_sec++; tmo.tv_nsec-=1000000000;}
		if (pthread_mutex_timedlock(&evt->mWait,&tmo)) return 2;  // timeout
		
		if (evt->iFlags & _EvtFlgManReset_)
		{	// Manual reset so other threads may run also
			pthread_mutex_unlock(&evt->mWait);	// other threads allowed
		}
		return 1;
	}
	return 0;
}
#endif	// __GLIBC_PREREQ(2,4) 
#endif	// __GLIBC_PREREQ

*/

#endif





#if defined(_WIN32)

void stlThreadSwitch(void)
{
	SwitchToThread();
}
void stlMsWait(int iMsCnt)
{
	Sleep(iMsCnt);
}
static void _initMutex(stlMutex *mux)
{
	static LONG mySpinLock=0;
	// initialize the Mutex in een thread save way
	if (mux->hHandle==NULL)
	{
		while(InterlockedExchange(&mySpinLock,1)) Sleep(1);
		if (mux->hHandle==NULL) 
			mux->hHandle=CreateMutex(NULL,0,NULL);
		mySpinLock=0;
	}
}
/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**       >0 (the mutex is already locked by another proces)
*/
int stlMutexTryLock(stlMutex *mux)
{
	_initMutex(mux);
	return 	WaitForSingleObject(mux->hHandle,0);
}
/* Try locking a mutex return immediately if the mutex is locked by another proces
**   mux   mutex data pointer
** Return 0 (we have acquired the lock on this mux)
**        1 (timeout)
**       >1 (other internal error)
*/
int stlMutexLockTo(stlMutex *mux,int iTimeOutMs)
{
	DWORD dwRes = 0;
	_initMutex(mux);
	dwRes= WaitForSingleObject(mux->hHandle,iTimeOutMs);
	if (dwRes == WAIT_TIMEOUT ) return 1;	// timeout
	if (dwRes == WAIT_OBJECT_0) return 0;	// ok locked
	return dwRes;
}

void stlMutexLock(stlMutex *mux)
{
	_initMutex(mux);
	WaitForSingleObject(mux->hHandle,INFINITE);
}
void stlMutexUnlock(stlMutex *mux)
{
	ReleaseMutex(mux->hHandle);
}
void stlMutexRelease(stlMutex *mux)
{
	HANDLE hnd;
	hnd=mux->hHandle;
	mux->hHandle=NULL;
	CloseHandle(hnd);
}
void stlMutexInit(stlMutex *mux)
{
	memset(mux,0,sizeof(struct stlMutex));
	_initMutex(mux);
}
static void winThread(struct threadIdData *TD)
{
	TD->iPid=stlGetThreadId();
	//printf("stlThreadStart %d (%s)",TD->iPid,TD->sName);
	stlThreadPriority(TD->iPrio);
	TD->start_address(TD->pData);
	memset(TD,0,sizeof(struct threadIdData));
	_endthread();
}
void stlThreadAbort(void)
{
	int i,iPid=stlGetThreadId();
	for (i=0;i<_MaxThread_;i++){
		if (tIDT[i].iPid==iPid){
			memset(&tIDT[i],0,sizeof(struct threadIdData));
			_endthread();
		}
	}
}


#define _EvtMagicNumber_	0x456A42BA
#define _EvtFlgOneShot_		0x0001		// Geeft aan dat eer een one shot aan de gang is

struct eventStruct{
	stlMutex  mux;	// deze is om te zorgen dat bij een one shot niet meerdere rondjes word gelopen
	HANDLE hEvent;	// Eigenlijke handle voor event
	int iFlags;
	int iMagic;
};

int stlEventCreate(int bManualReset,int bInitialState)
{
	struct eventStruct *evt;
	evt=malloc(sizeof(struct eventStruct));
	if (evt==NULL) return 0;
	memset(evt,0,sizeof(struct eventStruct));
	stlMutexInit(&evt->mux);
	evt->hEvent = CreateEvent(NULL,bManualReset,bInitialState,NULL);
	evt->iMagic = _EvtMagicNumber_;
	return (int)evt;
}

// Delete event
// return <>0 Ok
//          0 Error
int stlEventDelete(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		stlMutexRelease(&evt->mux);
		CloseHandle(evt->hEvent);
		memset(evt,0,sizeof(struct eventStruct));
		free(evt);
	}
	return 0;
}
// Activate event
// return <>0 Ok
//          0 Error
int stlEventSet(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
		return SetEvent(evt->hEvent);
	return 0;
}
// Activate event
// return <>0 Ok
//          0 Error
int stlEventReset(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
		return ResetEvent(evt->hEvent);
	return 0;
}
// Wacht tot een event actief word
// return 1 Event signaled
//        2 Timeout 
//        0 Error
int stlEventWaitTo(int iHdl,int iTimeOut)
{
	int iRes;
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		if (evt->iFlags & _EvtFlgOneShot_)
		{
			stlMutexLock(&evt->mux);
			stlMutexUnlock(&evt->mux);
		}
		iRes=WaitForSingleObject(evt->hEvent,iTimeOut);
		if (iRes==WAIT_OBJECT_0) return 1;	// event is raised
		if (iRes==WAIT_TIMEOUT ) return 2;	// timeout of wait
	}
	return 0;
}
int stlEventWait(int iHdl)
{
	return stlEventWaitTo(iHdl,INFINITE);
}




/* Activate all threads waiting for this event exactly 1 time
**  iHdl  Handle to event
*/
void stlEvent1Shot(int iHdl)
{
	int i;
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		evt->iFlags |= _EvtFlgOneShot_;
		stlMutexLock(&evt->mux);	// deze voorkomt dat een wachter meerdere rondjes draait tijdens de set event
		SetEvent(evt->hEvent);
		//stlThreadSwitch();			// op een "P4 HT" maakt deze niets uit. op een "Centrino Duo" wel
										// Op een P4 HT 3Ghz is het zonder deze switch sneller.
		ResetEvent(evt->hEvent);
		evt->iFlags &= ~_EvtFlgOneShot_;
		stlMutexUnlock(&evt->mux);
		for (i=0;i<100;i++) stlThreadSwitch();	// Dit moeten er 100 zijn anders mis je events als er 50 threads op dit event 
	}
}
void stlEvent1ShotFast(int iHdl)
{
	struct eventStruct *evt=(struct eventStruct *)iHdl;
	if ((evt)&&(evt->iMagic==_EvtMagicNumber_))
	{
		evt->iFlags |= _EvtFlgOneShot_;
		stlMutexLock(&evt->mux);		// deze voorkomt dat een wachter meerdere rondjes draait tijdens de set event
		SetEvent(evt->hEvent);
		ResetEvent(evt->hEvent);
		evt->iFlags &= ~_EvtFlgOneShot_;
		stlMutexUnlock(&evt->mux);
	}
}



// Sets thread priority (from -20 high to 20 low)
void stlThreadPriority(int iPrio)
{
	struct threadIdData *TD = _stlGetTD();
	if (iPrio> 20) iPrio= 20;
	if (iPrio<-20) iPrio=-20;
	if (TD) TD->iPrio = iPrio;

	     if (iPrio<=-20) iPrio=THREAD_PRIORITY_TIME_CRITICAL;
	else if (iPrio<=-17) iPrio=6;
	else if (iPrio<=-14) iPrio=5;
	else if (iPrio<=-11) iPrio=4;
	else if (iPrio<= -8) iPrio=3;
	else if (iPrio<= -5) iPrio=THREAD_PRIORITY_HIGHEST;		// 2
	else if (iPrio<= -2) iPrio=THREAD_PRIORITY_ABOVE_NORMAL;// 1
	else if (iPrio<   2) iPrio=THREAD_PRIORITY_NORMAL;      // 0
	else if (iPrio<   5) iPrio=THREAD_PRIORITY_BELOW_NORMAL;// 1
	else if (iPrio<   8) iPrio=THREAD_PRIORITY_LOWEST;      // 2
	else if (iPrio<  11) iPrio=-3;
	else if (iPrio<  14) iPrio=-4;
	else if (iPrio<  17) iPrio=-5;
	else if (iPrio<  20) iPrio=-6;
	else                 iPrio=THREAD_PRIORITY_IDLE;

	// LINUX values between -20(highest prio) and 20(lowest prio)
	// Windows               15                  -15
	SetThreadPriority(GetCurrentThread(),iPrio);
}
#endif



/* Starts a new thread return pid 
 *  thFnc  function to start the 
 *  Priority 0=Normal -20=Realtime 20=Idle
 */
int stlThreadStart(void(*thFnc)(void *),int prio,void *Param,const char *name)
{
	struct threadIdData *TD;
	TD=_stlNewThIdData();
	if (TD==NULL){
		printf("FATAL no more threads available\n");
		_strSafeExit(_StTlErrThreadMaxUsed_);
	}

	TD->iPrio        =prio;
	TD->start_address=thFnc;
	TD->pData        =Param;
	if (name) strncpy(TD->sName,name,sizeof(TD->sName)-1);
#if defined(__linux__) || defined(__APPLE__)
	{
		pthread_t th;
		pthread_create(&th,NULL,StartTask,TD);
	}
	
#elif defined (_WIN32)
	_beginthread(winThread,0,TD);
#else
#	error unsupported OS
#endif
	//stlMsWait(50);
	return 0;
}
