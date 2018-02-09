#include "stlHiresTimer.h"

#ifndef ITEMCOUNT
#  define ITEMCOUNT(a) (sizeof(a)/sizeof(a[0]))
#endif

stlHiresTimer::stlHiresTimer()
{
	enable = true;
	memset(sTimeD,0,sizeof(sTimeD));
	iTimeIx = 0;
	reset();
}
stlHiresTimer::~stlHiresTimer()
{
	for (int i=0;i<iTimeIx;i++) stlFree(sTimeD[i]);
}

void stlHiresTimer::reset()
{
	for (int i=0;i<iTimeIx;i++) stlFree(sTimeD[i]);
	memset(sTimeD,0,sizeof(sTimeD));
	memset(iTimeA,0,sizeof(iTimeA));	// eind tijden.
	memset(iTimeS,0,sizeof(iTimeS));	// start tijden (of vorige eind tijd)
	iTimeIx=0;
	iTimeA[iTimeIx++] = stlUsTimer(0);	// start tijd 0
}

void stlHiresTimer::start()
{
	if (!enable) return;
	if (iTimeIx+1 > (int)ITEMCOUNT(sTimeD)) return;
	iTimeS[iTimeIx] = stlUsTimer(0);
}

int stlHiresTimer::getUS()
{
	if (!enable) return 0;
	return stlUsTimer(iTimeA[0]);
}

void stlHiresTimer::stamp(const char *fmt,...)
{
	if (!enable) return;
	if (iTimeIx+1 > (int)ITEMCOUNT(sTimeD)) return;
	if (fmt)
	{
		va_list ap;
		va_start(ap,fmt);
		sTimeD[iTimeIx]=stlSetSta(fmt,ap);
		va_end(ap);
	}else
	{
		sTimeD[iTimeIx]=NULL;
	}
	iTimeA[iTimeIx++]=stlUsTimer(0);
}


void stlHiresTimer::reportAddTimings(STP sMsg)
{
	if (!enable) return;
	if (sMsg==NULL) return;
	int iTotTime=0;
	for (int i=1;i<iTimeIx;i++)
	{
		int iD1;	// verschil met vorige + totaal time
		if (iTimeS[i])
		{
			iD1 = iTimeA[i]-iTimeS[i];
		}else
		{
			iD1 = iTimeA[i]-iTimeA[i-1];
		}
		iTotTime += iD1;
		if (sTimeD[i]) stlAppendStf(sMsg," %s=%d.%d(%d)",sTimeD[i]->sBuf,iD1/1000,(iD1%1000)/100,iTotTime/1000);
		else           stlAppendStf(sMsg," T_%d=%d.%d(%d)",i+1,iD1/1000,(iD1%1000)/100,iTotTime/1000); 
	}
}
// beveiliging max 100 meldingen per sec.
static int reportCount = 0;
static int reportCleanMS = 0;
static int reportCleanup()	// return 1 als er geen reports meer mogen worden gedaan
{
	if (reportCount > 0)
	{
		unsigned int dif = stlMsTimer(reportCleanMS);		// laatste cleanup verschil
		int iCnt = reportCount - (dif / 10);
		if (iCnt < 0) iCnt = 0;								// niet onder 0 gaan
		if (iCnt != reportCount){ reportCleanMS += dif; reportCount = iCnt; }	// update only op verschil
	}
	if (reportCount > 100) return 1;
	reportCount++; 
	return 0;
}

STP stlHiresTimer::reportMsg(const char *fmt,...)
{
	if (!enable) return NULL;
	STP sMsg;
	va_list ap;
	if (reportCleanup()) return NULL;

	if (fmt==NULL) fmt="times(ms)";

	va_start(ap,fmt);
	sMsg=stlSetSta(fmt,ap);
	va_end(ap);
	reportAddTimings(sMsg);
	return sMsg;
}

void stlHiresTimer::report(const char *fmt,...)
{
	if (!enable) return;
	STP sMsg;
	va_list ap;
	if (reportCleanup()) return;

	if (fmt==NULL) fmt="times(ms)";

	va_start(ap,fmt);
	sMsg=stlSetSta(fmt,ap);
	va_end(ap);
	reportAddTimings(sMsg);
	printf("%s\n",sMsg->sBuf);
	stlFree(sMsg);
}
