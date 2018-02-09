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

#include <stdlib.h>
#include <vector>

#include "stl_thread.h"
#include "stl_str.h"

#if defined(__linux__)

#include <sys/time.h>

/************************************************************************/
/* Event afhandeling                                                    */
/************************************************************************/

#define _EvtMagicNumber_	0x456A42BA
#define _EvtFlgOneShot_		0x0001	// Geeft aan dat eer een one shot aan de gang is
#define _EvtFlgManReset_	0x0002	// als event manual reset is.
#define _EvtFlgSignaled_	0x0004	// als event gesignalled is

struct eventStruct{
	pthread_mutex_t mux,mWait;
	int iFlags;		// status flags
	int iMagic;
};

static std::vector <struct eventStruct*> eventDB;
static pthread_mutex_t eventDBmux = PTHREAD_MUTEX_INITIALIZER;


// Creëer een event
// return <>0 Ok
//          0 Error
int stlEventCreate(int bManualReset,int bInitialState)
{
	struct eventStruct *evt;
	evt=(struct eventStruct *)malloc(sizeof(struct eventStruct));
	if (evt==NULL) return 0;
	memset(evt,0,sizeof(struct eventStruct));
	pthread_mutex_init(&evt->mux  ,NULL);
	pthread_mutex_init(&evt->mWait,NULL);
	if (bManualReset ) evt->iFlags |= _EvtFlgManReset_;
	if (bInitialState) evt->iFlags |= _EvtFlgSignaled_;
	else               pthread_mutex_lock(&evt->mWait);
	evt->iMagic = _EvtMagicNumber_;
	
	pthread_mutex_lock(&eventDBmux);
	if (eventDB.size() < 2) eventDB.resize(5,NULL);
	int iLen = (int) eventDB.size();
	for (int i = 1; i < iLen; i++)
		if (eventDB[i] == NULL)
		{
			eventDB[i] = evt;
			pthread_mutex_unlock(&eventDBmux);
			return i;
		}
	eventDB.push_back(evt);
	pthread_mutex_unlock(&eventDBmux);
	return iLen;
}


// Delete event
// return <>0 Ok
//          0 Error
int stlEventDelete(int iHdl)
{
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
		eventDB[iHdl] = NULL;
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
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
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
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
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
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
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


/* Activate all threads waiting for this event exactly 1 time
**  iHdl  Handle to event
*/
void stlEvent1Shot(int iHdl)
{
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
		evt->iFlags |= _EvtFlgOneShot_;
		pthread_mutex_lock(&evt->mux);			// deze voorkomt dat een wachter meerdere rondjes draait tijdens de set event
		pthread_mutex_unlock(&evt->mWait);
		stlThreadSwitch();
		pthread_mutex_lock(&evt->mWait);
		evt->iFlags &= ~_EvtFlgOneShot_;
		pthread_mutex_unlock(&evt->mux);
		for (int i = 0; i < 3; i++) stlThreadSwitch();	// Dit moeten er 100 zijn anders mis je events als er 50 threads op dit event 
	}
}


#ifdef __GLIBC_PREREQ
#if __GLIBC_PREREQ(2,4)
int stlEventWaitTo(int iHdl,int iTimeOut)
{
	if ((iHdl > 0) && (iHdl < (int)eventDB.size()) && (eventDB[iHdl]) && (eventDB[iHdl]->iMagic == _EvtMagicNumber_))
	{
		struct eventStruct *evt = eventDB[iHdl];
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
#endif	// __linux__


/* event thread body's

static void msEventEntry(void *pUserData)
{
	stlCmsEvent *eb = (stlCmsEvent*)(pUserData);
	//if (!eb->enableEvent) return;
	eb->msEvent();
}

stlCmsEvent::stlCmsEvent():evInterval(1000),enabled(false)//  enableEvent(false)
{
	//stlTimeEventAddN(msEventEntry,1000,this,"stlCmsEvent");
}

stlCmsEvent::stlCmsEvent(int msInterval):evInterval(msInterval),enabled(false)//  enableEvent(false)
{
	//stlTimeEventAddN(msEventEntry,msInterval,this,"stlCmsEvent");
}

stlCmsEvent::stlCmsEvent(int msInterval,char *myName):evInterval(msInterval),enabled(false)// enableEvent(false)
{
	//stlTimeEventAddN(msEventEntry,msInterval,this,myName);
}

int stlCmsEvent::eventName(char *fmt,...)
{
	STP pName;
	va_list ap;
	va_start(ap,fmt);
	pName=stlSetSta(fmt,ap);
	va_end(ap);
	int iRes = stlTimeEventNameSet(msEventEntry,this,pName->sBuf);
	stlFree(pName);
	return iRes;
}

int stlCmsEvent::eventInterval(int msInterval)
{
	return stlTimeEventIntervalSet(msEventEntry,this,msInterval);
}
*/


/* Worker thread function body's

static void workerThreadEntry(void *pUserData)
{
	stlCThread *wt = (stlCThread*)(pUserData);
//	while (!wt->enableThread) stlMsWait(1);
	wt->worker();
}

stlCThread::stlCThread():threadPrio(1),enabled(false)	//  enableThread(false)
{
	//stlThreadStart(workerThreadEntry,1,this,"stlCThread");
}

stlCThread::stlCThread(int prio):threadPrio(prio), enabled(false)//  enableThread(false)
{
	//stlThreadStart(workerThreadEntry,prio,this,"stlCThread");
}

stlCThread::stlCThread(char *name):threadPrio(1),enabled(false)//  enableThread(false)
{
	//stlThreadStart(workerThreadEntry,1,this,name);
}

stlCThread::stlCThread(int prio,char *name):threadPrio(prio),enabled(false)//  enableThread(false)
{
	//stlThreadStart(workerThreadEntry,prio,this,name);
}

int stlCThread::threadName(char *fmt,...)
{
	STP pName;
	va_list ap;
	va_start(ap,fmt);
	pName=stlSetSta(fmt,ap);
	va_end(ap);
	int iRes = stlThreadPidNameSet((void *)workerThreadEntry,this,pName->sBuf);
	stlFree(pName);
	return iRes;
}

*/

