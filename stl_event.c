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
//#define _TIME_DEBUG


#include <stdlib.h>

#include "stl_str.h"
#include "stl_thread.h"

#ifdef _TIME_DEBUG
#  include <intrin.h>
#endif

#ifdef _WIN32
#  pragma comment(lib,"winmm.lib")
#endif

#ifdef __linux__
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#  include <arpa/inet.h>
#endif
#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
#endif


typedef struct _eventStruct 
{
	stlMutex runMux;
	stlTimeEvent pEventFx;		// functie pointers voor timer events
	void     *pEventData;		// pointers voor user data van timers
	int       iMsCounter;		// word gebruikt om verstreken tijd te berekenen
	int       iIntervalTime;	// interval voor de timer events
	volatile int iRunCnt;		// aantal keer starten zodra dat kan
	struct _eventStruct *next;
	char      sName[50];
#ifdef _TIME_DEBUG
	// debug info
	DWORD64 cpuTime;			// totaal aantal CPU ticks
	int     cpuRun;				// totaal aantal keer aangeroepen
#endif
}_eventData;

static _eventData *evFirst=NULL;
static stlMutex evMux=stlMutexInitializer;

static stlMutex tmrMux=stlMutexInitializer;
static int iUsTimer=0;

extern int stlPrintfBackgroundActive;


#ifdef _TIME_DEBUG
void stlTimeEventReport()
{
	_eventData *ed=evFirst;
	printf("timeEvent;cnt;cpu;name;total");
	while (ed)
	{
		int iAvg=0;
		if (ed->cpuRun) iAvg=ed->cpuTime / ed->cpuRun;
		printf("timeEvent;%d;%d;%s;%l64d",ed->cpuRun,iAvg,ed->sName,ed->cpuTime);
		ed->cpuRun=0;
		ed->cpuTime=0;
		ed=ed->next;
	}
}
#endif

int usEventWarnThreshold = 3000;
static int _eventCpuPerMs = 2500000;	// standaard 2.5 GHz CPU
static int _eventRunThread_ = 1;
static int _eventThreadCount_ = 0;
static void _EventTask(void *pDummy)
{
	int _eventRateLimit = 0;
	int eventWarn = usEventWarnThreshold;
	char *namePtr;
	_eventData *ed;
	int iDone;
	iUsTimer=stlUsTimer(0);
	namePtr = (char*)stlThreadName();

#ifdef _DEBUG
	if (eventWarn < 4000) eventWarn = 4000;
#endif // _DEBUG
#ifdef __arm__
	if (eventWarn < 8000) eventWarn = 8000;
#endif	// __arm__


	while (_eventRunThread_)
	{
#ifdef __arm__
		stlMsWait(5);
#else
		stlMsWait(1);
#endif
		if (_eventRateLimit > 0) _eventRateLimit--;				// elke ms terug tellen

		if (stlMutexTryLock(&tmrMux)==0)
		{
			iDone=stlUsTimer(iUsTimer);
			if ((iDone > eventWarn) && (_eventRateLimit <= 1000))
			{
				printf("#25# ms event %d us",iDone);
				_eventRateLimit += stlPrintfBackgroundActive ? 10 : 100;	// background print max 100(10ms) per sec normal max 10(100ms) per sec
			}
			iDone=iDone/1000;
			if ((iDone<0)||(iDone>100)){iDone=1; iUsTimer=stlUsTimer(0);}	// reset als er wazige tijden terug komen
			else                       {iUsTimer += iDone*1000;}			// verhoog start timer
			ed = evFirst;
			while (ed)
			{
				if ((ed->pEventFx)&&(ed->iIntervalTime>0))
				{
					ed->iMsCounter-=iDone;								// Werkelijk verstreken ms in mindering brengen op wachttijd
					while (ed->iMsCounter<=0)
					{
						ed->iRunCnt++;
						ed->iMsCounter+=ed->iIntervalTime;				// corrigeer met verstreken tijd
					}
				}
				ed=ed->next;
			}
			stlMutexUnlock(&tmrMux);
		}

		ed=evFirst;
		while (ed)
		{
			stlTimeEvent Fx = ed->pEventFx;
			if ((Fx)&&(ed->iRunCnt > 0)&&(stlMutexTryLock(&ed->runMux)==0))
			{
#ifdef __arm__
#else
				__int64 iStart = stlCpuTimer64(0);
#endif
				if (ed->iRunCnt <= 0)
				{
					stlMutexUnlock(&ed->runMux);
					ed=ed->next;
					continue;
				}

				//int runCnt,runSave;
				strncpy(namePtr,ed->sName,50-1);		// set name of current thread to event name (stl thread limit 50 chars)
				
				if (ed->iRunCnt > 7) ed->iRunCnt = 7;	// max 7 slagen achter lopen
				if (ed->iRunCnt > 0){
					ed->iRunCnt--;
					Fx(ed->pEventData);					// voor timer event uit
				}
				if (ed->iRunCnt > 0){					// max 2 maal in 1 loop
					ed->iRunCnt--;
					Fx(ed->pEventData);					// voor timer event uit
				}
#ifdef __arm__
#else
				iStart = stlCpuTimer64(iStart);
#endif
				stlMutexUnlock(&ed->runMux);
#ifdef __arm__
#else
				if ((iStart > _eventCpuPerMs * 4) && (_eventRateLimit <= 1000))
				{
					int ms10 = (int)(iStart * 10 / _eventCpuPerMs);
					int ms = ms10 / 10;
					ms10 -= ms * 10;
#ifndef _DEBUG
					printf("#25# %d ms event '%s' took %d.%d ms",ed->iIntervalTime,ed->sName,ms,ms10);
#endif
					_eventRateLimit += stlPrintfBackgroundActive ? 10 : 100;	// background print max 100(10ms) per sec normal max 10(100ms) per sec
				}
#endif
			}
			ed=ed->next;
		}
	}
}
static void _initEvents(void)
{
	static stlMutex mux = stlMutexInitializer;
	static volatile int iDone=0;
	stlMutexLock(&mux);
	if (iDone){
		stlMutexUnlock(&mux);
		return;
	}
	iDone=1;
#ifdef _WIN32
	timeBeginPeriod(1);	// set system clock naar 1ms i.p.v. 10ms
#endif
	_eventCpuPerMs = stlCpuPerMs(0);
	stlThreadStart(_EventTask,-20,NULL,"EventTask");
	stlThreadStart(_EventTask,-20,NULL,"EventTask");
	stlThreadStart(_EventTask,-20,NULL,"EventTask");
	_eventThreadCount_ = 3;
	stlMutexUnlock(&mux);
}

void stlTimeEventsExit()
{
	_eventRunThread_ = 0;
}

/* adjust / update name of ms event functie
*/
int stlTimeEventNameSet(stlTimeEvent timeFx,void *pUserData,const char *sName)
{
	_eventData *ed;
	if ((sName == NULL) || (sName[0] == 0)) return -1;
	_initEvents();
	stlMutexLock(&evMux);
	ed=evFirst;
	while (ed)
	{
		if ((ed->pEventFx == timeFx) && (ed->pEventData == pUserData))
		{
			strncpy(ed->sName,sName,sizeof(ed->sName)-1);
			stlMutexUnlock(&evMux);
			return 0;
		}
		ed=ed->next;
	}
	stlMutexUnlock(&evMux);
	return -1;
}

/* adjust interval tijd van een ms event functie
*/
int stlTimeEventIntervalSet(stlTimeEvent timeFx,void *pUserData,int iMsInterval)
{
	_eventData *ed;
	_initEvents();
	if (iMsInterval<=0) iMsInterval=1;
	stlMutexLock(&evMux);
	ed=evFirst;
	while (ed)
	{
		if ((ed->pEventFx == timeFx) && (ed->pEventData == pUserData))
		{
			ed->iMsCounter    = iMsInterval;	// restart timer
			ed->iIntervalTime = iMsInterval;	// set reload value
			stlMutexUnlock(&evMux);
			return 0;
		}
		ed=ed->next;
	}
	stlMutexUnlock(&evMux);
	return -1;
}

/* Laat een bepaald functie met bepaalde interval uitvoeren.
** Deze interval is exact en niet onderhevig aan systeem belasting max jitter 2ms.
**  timeFx     functie welke met interval uitgevoerd moet worden.
**  msInterval interval in milliseconden waarin deze functie aangeroepen moet worden.
**  pUserData  user defined data word aan functie weer meegegeven.
**  sName      name of the thread during execution (for debug simplification)
** Return 0 Ok
**       -1 out of memory
*/
int stlTimeEventAddN(stlTimeEvent timeFx,int iMsInterval,void *pUserData,const char *sName)
{
	int iCount = 0;
	_eventData *ed;
	_initEvents();

	if (iMsInterval<=0) iMsInterval=1;

	stlMutexLock(&evMux);
	// look first if entry already in table
	ed=evFirst;
	while (ed)
	{
		if ((ed->pEventFx == timeFx) &&
			(ed->pEventData == pUserData))
		{
			stlMutexUnlock(&evMux);
			return 0;					// entry is there already
		}
		ed=ed->next;
	}
	// look for empty slot in chain
	ed=evFirst;
	while (ed)
	{
		iCount++;
		if (ed->pEventFx == NULL)
		{
			strncpy(ed->sName,sName,sizeof(ed->sName)-1);
			ed->iRunCnt = 0;
			ed->iMsCounter =iMsInterval;
			ed->iIntervalTime =iMsInterval;
			ed->pEventData=pUserData;
			ed->pEventFx=timeFx;
			stlMutexUnlock(&evMux);
			return 0;					// entry is there already
		}
		ed=ed->next;
	}

	// nieuwe struct pointer maken en leeg maken
	ed=malloc(sizeof(_eventData));
	if (ed==NULL) return -1;
	memset(ed,0,sizeof(_eventData));

	// data vullen met info
	strncpy(ed->sName,sName,sizeof(ed->sName)-1);
	ed->iMsCounter =iMsInterval;
	ed->iIntervalTime =iMsInterval;
	ed->pEventData=pUserData;
	ed->pEventFx=timeFx;

	stlMutexInit(&ed->runMux);

	ed->next=evFirst;
	evFirst=ed;

	/*
	if (iCount > 16)
	{
		int ntc = (iCount / 8) + 1;
		if (ntc > _eventThreadCount_)
		{
			_eventThreadCount_++;
			stlThreadStart(_EventTask,-20,NULL,"EventTask");
		}
	}
	*/

	stlMutexUnlock(&evMux);

	//printf("stlTimeEventAddN %d %d %d",iCount,iMsInterval,_eventThreadCount_);

	return 0;
}

/* Laat een bepaald functie met bepaalde interval uitvoeren.
** Deze interval is exact en niet onderhevig aan systeem belasting max jitter 2ms.
**  timeFx     functie welke met interval uitgevoerd moet worden.
**  msInterval interval in milliseconden waarin deze functie aangeroepen moet worden.
**  pUserData  user defined data word aan functie weer meegegeven.
** Return 0 Ok
**       -1 out of memory
*/
int stlTimeEventAdd(stlTimeEvent timeFx,int iMsInterval,void *pUserData)
{
	return stlTimeEventAddN(timeFx,iMsInterval,pUserData,"msEventTask");
}

/* Verwijder een event uit de timer event list
**  timeFx     pointer naar event functie
**  pUserData  als deze NULL is worden alle verwijzingen naar deze functie verwijderd.
**             indien deze een waarde bevat word alleen de functie met deze waarde verwijderd
** Return 0 Ok
*/
int stlTimeEventDel(stlTimeEvent timeFx,void *pUserData)
{
	_eventData *ed;

	ed=evFirst;
	while (ed)
	{
		if (ed->pEventFx == timeFx)
		{
			if (pUserData)			// als user-data is mee gegeven deze ook controleren.
			{
				if (ed->pEventData == pUserData) ed->pEventFx=NULL;	// zet remove flag als deze gelijk is
			}else
			{
				ed->pEventFx=NULL;		// zet remove flag voor deze event
			}
		}
		ed=ed->next;
	}
	return 0;
}

#define _MaxSetDataLen_		50
#define _MaxSetDataItems_	1000

struct timeSetVar{
	void *pData;
	int   iLen;
	int   iBitNr,iSetClr,iBitMsk;
	int   iMsWait;
	char  sData[_MaxSetDataLen_];
	struct timeSetVar *next;
};

static struct timeSetVar *tsvDB  =NULL;
static struct timeSetVar *tsvRoot=NULL;
static stlMutex tsvMux = stlMutexInitializer;



static void setClearBit(unsigned char *pdata,int iBitNr,int iSetClr)
{
	pdata += (iBitNr / 8);
	if (iSetClr)
		pdata[0] |= (1 << (iBitNr & 7));
	else
		pdata[0] &= ((1 << (iBitNr & 7)) ^ 0xFF);
}

static void setClearMask(void *pdata,int iBitMask,int iSetClr,int iDlen)
{
	if ((iBitMask == 0) || (pdata == NULL)) return;
	if (iDlen == 1)
	{
		unsigned char *data = (unsigned char *)pdata;
		if (iSetClr)
		{
			if ((*data & iBitMask) != iBitMask)	// indien nog niet gezet zijn
				*data |= iBitMask;
		}else
		{
			if (*data & iBitMask)					// indien er nog bits gezet zijn.
				*data &= ~iBitMask;
		}
	}else if (iDlen == 2)
	{
		unsigned short *data = (unsigned short *)pdata;
		if (iSetClr)
		{
			if ((*data & iBitMask) != iBitMask)	// indien nog niet gezet zijn
				*data |= iBitMask;
		}else
		{
			if (*data & iBitMask)					// indien er nog bits gezet zijn.
				*data &= ~iBitMask;
		}
	}else if (iDlen == 4)
	{
		unsigned int *data = (unsigned int *)pdata;
		if (iSetClr)
		{
			if ((*data & iBitMask) != iBitMask)	// indien nog niet gezet zijn
				*data |= iBitMask;
		}else
		{
			if (*data & iBitMask)					// indien er nog bits gezet zijn.
				*data &= ~iBitMask;
		}
	} else _lxBreak_();
}

static void _stlTimeSetEvent(void *pUserData)
{
	struct timeSetVar *tv = tsvRoot,*tvl=NULL;
	while (tv)
	{
		tv->iMsWait--;
		if (tv->iMsWait <= 0)
		{
			if (tv->iBitMsk)
			{
				setClearMask(tv->pData,tv->iBitMsk,tv->iSetClr,tv->iLen);
			}else
			if (tv->iBitNr >= 0)
			{
				setClearBit(tv->pData,tv->iBitNr,tv->iSetClr);
			}else{
				memcpy(tv->pData,tv->sData,tv->iLen);
			}
			if (tvl){tvl->next = tv->next; tv->pData=NULL; tv=tvl->next; continue;}	// moet in deze volgorde anders gaat het fout met threading
			// we waren root record extra zorg nodig om concurrency te bewaken.
			stlMutexLock(&tsvMux);
			if (tv==tsvRoot)		// we zijn nog steeds root record.
			{
				tsvRoot=tv->next;	// set root to next item.
				tv->pData=NULL;		// release item
				tv=tsvRoot;			// next item is root again.
			}else
			{
				tvl=tsvRoot;		// look for our item 
				while (tvl)
				{
					if (tvl->next==tv)
					{
						tvl->next = tv->next;
						tv->pData=NULL;
						tv=tvl->next;
						break;
					}
					tvl=tvl->next;
				}
			}
			stlMutexUnlock(&tsvMux);
			continue;
		}
		tvl=tv;
		tv=tv->next;
	}
}

static void _stlTimeSetInit(void)
{
	if (tsvDB) return;
	tsvDB=malloc(sizeof(struct timeSetVar) * _MaxSetDataItems_);
	memset(tsvDB,0,sizeof(struct timeSetVar) * _MaxSetDataItems_);
	stlTimeEventAdd(_stlTimeSetEvent,1,NULL);
}

void *stlTimeSetDataTripAdr = NULL;

/* Set integer to specific value after specified time
**   pData    pointer to set value in
**   pValue   Value to write to data after delay.
**   iValLen  number of bytes in pValue
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -1 to many bytes to write
**       -2 no more room in database
*/
int stlTimeSetData(void *pData,void *pValue,int iValLen,int iMsDelay)
{
	int i,iRes=-2;
	
	if (iValLen<=0) return -1;
	if (iValLen>_MaxSetDataLen_) return -1;

#ifdef _WIN32
	if (pData == stlTimeSetDataTripAdr)
	{
		__debugbreak();
	}
#endif

	stlMutexLock(&tsvMux);
	_stlTimeSetInit();
	for (i=0;i<_MaxSetDataItems_;i++)
	{
		if (tsvDB[i].pData==NULL)
		{
			memset(&tsvDB[i],0,sizeof(tsvDB[i]));
			tsvDB[i].iBitNr = -1;
			tsvDB[i].pData  = pData;
			tsvDB[i].iLen   = iValLen;
			tsvDB[i].iMsWait = iMsDelay;
			memcpy(tsvDB[i].sData,pValue,iValLen);
			tsvDB[i].next = tsvRoot;
			tsvRoot=&tsvDB[i];
			iRes=0;
			break;
		}
	}
	stlMutexUnlock(&tsvMux);
	return iRes;
}


static int stlTimeSetDataBit(void *pData,int bitNr,int iValLen,int iMsDelay,int iSetClr)
{
	int i,iRes=-2;

	if ((bitNr < 0) || (bitNr >= iValLen * 8)) return -1;
	if (iValLen<=0) return -1;
	if (iValLen>_MaxSetDataLen_) return -1;

	stlMutexLock(&tsvMux);
	_stlTimeSetInit();
	for (i=0;i<_MaxSetDataItems_;i++)
	{
		if (tsvDB[i].pData==NULL)
		{
			memset(&tsvDB[i],0,sizeof(tsvDB[i]));
			tsvDB[i].iBitNr  = bitNr;
			tsvDB[i].iSetClr = iSetClr;
			tsvDB[i].pData   = pData;
			tsvDB[i].iLen    = iValLen;
			tsvDB[i].iMsWait = iMsDelay;
			tsvDB[i].next = tsvRoot;
			tsvRoot=&tsvDB[i];
			iRes=0;
			break;
		}
	}
	stlMutexUnlock(&tsvMux);
	return iRes;
}
int stlTimeSetClrDataMask32(int *pData,int bitMask,int iMsDelay,int iSetClr)
{
	int i,iRes=-2;
	if ((pData == NULL) || (bitMask == 0)) return -1;

	stlMutexLock(&tsvMux);
	_stlTimeSetInit();
	for (i=0;i<_MaxSetDataItems_;i++)
	{
		if (tsvDB[i].pData==NULL)
		{
			memset(&tsvDB[i],0,sizeof(tsvDB[i]));
			tsvDB[i].iBitMsk = bitMask;
			tsvDB[i].iSetClr = iSetClr;
			tsvDB[i].pData   = pData;
			tsvDB[i].iLen    = 4;
			tsvDB[i].iMsWait = iMsDelay;
			tsvDB[i].next = tsvRoot;
			tsvRoot=&tsvDB[i];
			iRes=0;
			break;
		}
	}
	stlMutexUnlock(&tsvMux);
	return iRes;
}
int stlTimeSetClrDataMask16(unsigned short *pData,unsigned short bitMask,int iMsDelay,int iSetClr)
{
	int i,iRes=-2;
	if ((pData == NULL) || (bitMask == 0)) return -1;

	stlMutexLock(&tsvMux);
	_stlTimeSetInit();
	for (i=0;i<_MaxSetDataItems_;i++)
	{
		if (tsvDB[i].pData==NULL)
		{
			memset(&tsvDB[i],0,sizeof(tsvDB[i]));
			tsvDB[i].iBitMsk = bitMask;
			tsvDB[i].iSetClr = iSetClr;
			tsvDB[i].pData   = pData;
			tsvDB[i].iLen    = 2;
			tsvDB[i].iMsWait = iMsDelay;
			tsvDB[i].next = tsvRoot;
			tsvRoot=&tsvDB[i];
			iRes=0;
			break;
		}
	}
	stlMutexUnlock(&tsvMux);
	return iRes;
}



/* Set integer to specific value after specified time
**   pDstVal  pointer to integer to set value in
**   setVal   Value to write to integer.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetPtr(void **pDst,void *pSet,int iMsDelay)
{
	return stlTimeSetData(pDst,&pSet,sizeof(void*),iMsDelay);
}
int stlTimeSetInt(int *pDstVal,int setVal,int iMsDelay)
{
	return stlTimeSetData(pDstVal,&setVal,sizeof(setVal),iMsDelay);
}
int stlTimeSetIntBit(int *pDstVal,int setBit,int iMsDelay)
{
	return stlTimeSetDataBit(pDstVal,setBit,sizeof(int),iMsDelay,1);
}
int stlTimeClrIntBit(int *pDstVal,int clrBit,int iMsDelay)
{
	return stlTimeSetDataBit(pDstVal,clrBit,sizeof(int),iMsDelay,0);
}

/* Set byte to specific value after specified time
**   pDstVal  pointer to byte to set value in
**   setVal   Value to write to byte.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetByte(char *pDstVal,char setVal,int iMsDelay)
{
	return stlTimeSetData(pDstVal,&setVal,sizeof(setVal),iMsDelay);
}
/* Set float to specific value after specified time
**   pDstVal  pointer to float to set value in
**   setVal   Value to write to float.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetFloat(float *pDstVal,float setVal,int iMsDelay)
{
	return stlTimeSetData(pDstVal,&setVal,sizeof(setVal),iMsDelay);
}
/* Set double to specific value after specified time
**   pDstVal  pointer to double to set value in
**   setVal   Value to write to double.
**   iMsDelay Number of ms to wait before setting the value
** Return 0 Ok
**       -2 no more room in database
*/
int stlTimeSetDouble(double *pDstVal,double setVal,int iMsDelay)
{
	return stlTimeSetData(pDstVal,&setVal,sizeof(setVal),iMsDelay);
}
