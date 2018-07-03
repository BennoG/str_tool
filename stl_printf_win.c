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
#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "stl_str.h"

static char appFn[150]="";
static char msgFn[150]="";
static HWND hDlgWnd=0;
static int  iListID=0;
static FILE *fileOut=(FILE*)-1;
static void (*printfHandler)(STP)=NULL;	
static void (*printfHandler2)(struct stlLogInfoStruct *)=NULL;	

int printfLogRotCount = -1;
int printfLogRotSize  = 4000000;

static int showMsTimes = 0;

void stlPrintfSetMsTimes(int iOn)
{
	showMsTimes = iOn;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

typedef struct{
	STP sMsg;		// hier komt melding in te staan
	char sNam[50];	// naam van de thread
	SYSTEMTIME st;	// hier datum en tijd van melding
	unsigned int msTime;	// bericht tijd in ms
}warnInfo;

static void procesWarnInfo(warnInfo *WI)
{
	static stlMutex muxf=stlMutexInitializer;	// For stdout writing
	static stlMutex mux =stlMutexInitializer;	// For writing to file
	int i,nri;
	STP Tm1,Tm2;
	FILE *f1=NULL;

	stlRemoveMult(WI->sMsg,"\r\t");
	stlConvert(WI->sMsg,10,_D1);
	nri=stlCountChr(WI->sMsg->sBuf,_D1)+1;

	if (fileOut==(FILE*)-1) fileOut=stdout;

	if (printfHandler) printfHandler(WI->sMsg);
	if (printfHandler2)
	{
		struct stlLogInfoStruct LI;
		memset(&LI,0,sizeof(LI));
		LI.sMsg = WI->sMsg;
		memcpy(LI.sNam,WI->sNam,sizeof(LI.sNam));
		LI.datY = WI->st.wYear;
		LI.datM = WI->st.wMonth;
		LI.datD = WI->st.wDay;
		LI.timH = WI->st.wHour;
		LI.timM = WI->st.wMinute;
		LI.timS = WI->st.wSecond;
		LI.msTime = WI->msTime;
		printfHandler2(&LI);
		WI->sMsg = LI.sMsg;
	}

	if (fileOut){
		for (i=1;i<=nri;i++){
			Tm1=stlGetStr(WI->sMsg,i,0,0);
			if (Tm1->iLen){
				stlMutexLock(&muxf);
				fwrite(Tm1->sBuf,1,Tm1->iLen,fileOut);
				if (fileOut == stdout)
					fwrite("\n",1,1,fileOut);
				else
					fwrite("\r\n",1,2,fileOut);
				stlMutexUnlock(&muxf);
			}
			stlFree(Tm1);
		}
	}

	if (msgFn[0]){
		if (WI->msTime)
			Tm1 = stlSetStf("\r\n%02d-%02d %02d:%02d:%02d;%07d;%s[%s]; ",WI->st.wMonth,WI->st.wDay,WI->st.wHour,WI->st.wMinute,WI->st.wSecond,WI->msTime,appFn,WI->sNam);
		else
			Tm1=stlSetStf("\r\n%04d-%02d-%02d %02d:%02d:%02d %s[%s]: ",WI->st.wYear,WI->st.wMonth,WI->st.wDay,WI->st.wHour,WI->st.wMinute,WI->st.wSecond,appFn,WI->sNam);
		// Voeg aan messages bestand toe
		for (i=0;i<5;i++){
			stlMutexLock(&mux);
			f1=fopen(msgFn,"ab");
			if (f1) break;
			stlMutexUnlock(&mux);
			stlMsWait(10);
		}
		if (f1){
			for (i=1;i<=nri;i++){
				Tm2=stlGetStr(WI->sMsg,i,0,0);
				if (Tm2->iLen){
					fwrite(Tm1->sBuf,1,Tm1->iLen,f1);
					fwrite(Tm2->sBuf,1,Tm2->iLen,f1);
				}
				stlFree(Tm2);
			}
			i=ftell(f1);	// Grootte van het bestand.
			fclose(f1);
			if (i > printfLogRotSize){	// Meer dan 4 Mb (backup naar .old)
				if (printfLogRotCount>=0)
				{
					while (1)
					{
						Tm2 = stlSetStf("%s.%04d",msgFn,printfLogRotCount++);
						f1 = fopen(Tm2->sBuf,"rb");
						if (f1 == NULL) break;
						fclose(f1);
						stlFree(Tm2);
					}
				}else 
					Tm2 = stlSetStf("%s.old",msgFn);
				unlink(Tm2->sBuf);
				rename(msgFn,Tm2->sBuf);
				stlFree(Tm2);
			}
			stlMutexUnlock(&mux);
		}
		stlFree(Tm1);
	}
	stlFree(WI->sMsg);
	memset(&WI,0,sizeof(WI));
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

/* printf data in separate thread
*/


#define _MaxWarnHist_	2000
static warnInfo warnDb[_MaxWarnHist_];
static stlMutex warnMux=stlMutexInitializer;
static int      warnHead = 0;
static int      warnTail = 0;

int printfDroppedMsgs = 0;
int stlPrintfBackgroundActive=0;
static int  stlPrintThreadFlg=0;
static void _stlPrintThread(void *pUserData)
{
	warnInfo WI;
	stlPrintfBackgroundActive=1;
	while (1)
	{
		if (warnHead == warnTail){
			if (stlPrintThreadFlg==2)
			{
				stlPrintThreadFlg=0;
				return;
			}
			stlMsWait(20); 
			continue;
		}

		// bericht uit de ring buffer halen
		stlMutexLock(&warnMux);
		memcpy(&WI,&warnDb[warnTail],sizeof(WI));
		memset(&warnDb[warnTail],0,sizeof(WI));
		stlMutexUnlock(&warnMux);
		warnTail = (warnTail+1) % _MaxWarnHist_;

		procesWarnInfo(&WI);
	}
}
static int queueWarnGetFree()
{
	int iUsed = (warnHead - warnTail);
	if (iUsed < 0) iUsed += _MaxWarnHist_;
	return _MaxWarnHist_ - iUsed;
}
static void _stlPrintThreadAbort(void)
{
	int i;
	stlPrintThreadFlg=2;
	stlPrintfBackgroundActive=0;
	for (i=0;i<500;i++)
	{
		if (stlPrintThreadFlg==0) break;
		stlMsWait(10);
	}
}

/* Do printf screen and file handling in a separate low priority thread
*/
void stlPrintfThread()
{
	if (stlPrintThreadFlg) return;
	stlPrintThreadFlg=1;
	memset(warnDb,0,sizeof(warnDb));
	stlThreadStart(_stlPrintThread,20,NULL,"print Thread");
	atexit(_stlPrintThreadAbort);
}
/************************************************************************/
/*                                                                      */
/************************************************************************/

void stlPrintfSetFile(FILE *fOut)
{
	fileOut=fOut;
}
void stlPrintfSetHdlg(HWND hWnd,int iLstID)
{
	hDlgWnd=hWnd;
	iListID=iLstID;
}
void stlPrintfSetLogFn(const char *sLogFn)
{
	if (sLogFn){
		memset(msgFn,0,sizeof(msgFn));
		strncpy(msgFn,sLogFn,sizeof(msgFn)-1);
	}else{
		msgFn[0]=0;
	}
}

void stlPrintfSetAppName(const char *sName)
{
	if (sName){
		memset(appFn,0,sizeof(appFn));
		strncpy(appFn,sName,sizeof(appFn)-1);
	}else{
		appFn[0]=0;
	}
}

/* Set user handler for printf output of application
** All other handling is ignored if this 
*/
void stlPrintfHandler(void (*userHandler)(STP))
{
	printfHandler = userHandler;
}
void stlPrintfHandler2(void (*userHandler)(struct stlLogInfoStruct*))
{
	printfHandler2 = userHandler;
}



int stlPrintf(const char *format , ... )
{
	warnInfo WI;
	va_list args;

	memset(&WI,0,sizeof(WI));
	va_start (args, format);
	WI.sMsg=stlSetSta(format,args);
	va_end   (args);
	if (WI.sMsg==NULL) return 0;
	strncpy(WI.sNam,stlThreadName(),sizeof(WI.sNam)-1);
	if (showMsTimes)
		WI.msTime = stlMsTimer(0);	// tijd in ms
	GetLocalTime(&WI.st);			// gets current local time

	if (stlPrintThreadFlg==1)
	{
		stlMutexLock(&warnMux);
		if (queueWarnGetFree() > 10)
		{
			if (warnDb[warnHead].sMsg) stlFree(warnDb[warnHead].sMsg);
			memcpy(&warnDb[warnHead],&WI,sizeof(WI));
			warnHead = (warnHead + 1) % _MaxWarnHist_;
		}else
		{
			printfDroppedMsgs++;
			stlFree(WI.sMsg);
		}
		stlMutexUnlock(&warnMux);
	}else
	{
		procesWarnInfo(&WI);
	}
	return 0;
}

#endif // _WIN32


