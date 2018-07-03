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
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#  include <syslog.h>
#  include <syscall.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <stdarg.h>
#  include <time.h>
#endif

#include "stl_str.h"


#ifdef _WIN32
static HWND hWndMe=NULL;
static void myExit(void)
{
	if (hWndMe){
		ShowWindow(hWndMe,SW_SHOW);
		ShowWindow(hWndMe,SW_SHOWNORMAL);
	}
}

HWND stlGetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024 // Buffer size for console window titles.
	HWND hwndFound;         // This is what is returned to the caller.
	char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
	// WindowTitle.
	char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
	// WindowTitle.

	// Fetch current window title.
	if (GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE)==0) return NULL;

	// Format a "unique" NewWindowTitle.
	wsprintf(pszNewWindowTitle,"%d/%d",GetTickCount(),GetCurrentProcessId());

	// Change current window title.
	if (SetConsoleTitle(pszNewWindowTitle)==0) return NULL;

	// Ensure window title has been updated.
	Sleep(40);

	// Look for NewWindowTitle.
	hwndFound=FindWindow(NULL, pszNewWindowTitle);

	// Restore original window title.
	SetConsoleTitle(pszOldWindowTitle);

	return(hwndFound);
}



/* Daemonize this window set all output to messages.txt
** 
*/
void stlDeamonize(void)
{
	if (hWndMe) return;	// already daemon
	hWndMe=stlGetConsoleHwnd();
	if (hWndMe==NULL) hWndMe=GetForegroundWindow();
	if (hWndMe){
		ShowWindow(hWndMe,SW_MINIMIZE);
		ShowWindow(hWndMe,SW_HIDE);
		atexit(myExit);
	}
}

static void fixLockName(char *sLock,const char *sFullName,int maxLen)
{
	int i,j;
	memset(sLock,0,maxLen);
	maxLen-=3;
	for (i=j=0;j<maxLen;i++){
		char ch=sFullName[i];
		if (ch==0) break;
		if ((ch>='0')&&(ch<='9')){sLock[j++]=ch; continue;}
		if ((ch>='a')&&(ch<='z')){sLock[j++]=ch; continue;}
		if ((ch>='A')&&(ch<='Z')){sLock[j++]=ch; continue;}
	}
	sLock[j]=0;
	return;
}
/* sets system wide lock so application can only be running once
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int setPidLock(const char *_LockFile_)
{
	char sLock[100];
	HANDLE hMux;
	fixLockName(sLock,_LockFile_,sizeof(sLock));
	hMux=CreateMutex(NULL,FALSE,sLock);
	if (hMux==NULL) return 1;	// kan geen lock handle maken.
	if (GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(hMux);return 1;} // lock bestaat al
	return 0;
}
/* Test of application already running
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int testPidLock(const char *_LockFile_)
{
	char sLock[100];
	HANDLE hMux;
	fixLockName(sLock,_LockFile_,sizeof(sLock));
	hMux=CreateMutex(NULL,FALSE,sLock);
	if (hMux==NULL) return 1;	// kan geen lock handle maken.
	if (GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(hMux);return 1;} // lock bestaat al
	CloseHandle(hMux);
	return 0;
}
#endif




#if defined(__linux__)

#define _NEW_HDLR_	// 2016-03-10 nieuwe manier van printf ahfandelen 

/*
* Change into a daemon.
* Get rid of the stdio streams, and disassociate from the original
* controlling terminal, and become a group leader.
*/
int stlPrintfBackgroundActive=0;
int is_daemon=0;

int printfLogRotCount = -1;
int printfLogRotSize  = 4000000;

static int   showMsTimes = 0;
static void (*printfHandler)(STP)=NULL;	
static void (*printfHandler2)(struct stlLogInfoStruct *)=NULL;	
static char msgFn[150]="";
static char appFn[150]="";

static void initWarnInfo(struct stlLogInfoStruct *wi)
{
	if (wi == NULL) return;
	memset(wi,0,sizeof(struct stlLogInfoStruct));
	if (showMsTimes)
		wi->msTime = stlMsTimer(0);	// tijd in ms
	else{
		struct tm *tm=NULL;
		time_t ti;
		ti=time(NULL);		// _time64       On Windows macro
		tm=localtime(&ti);	// _localtime64  On Windows macro
		if (tm){
			wi->timH = tm->tm_hour;
			wi->timM = tm->tm_min;
			wi->timS = tm->tm_sec;
			wi->datY = tm->tm_year;
			wi->datM = tm->tm_mon + 1;
			wi->datD = tm->tm_mday;
		}
	}
#ifdef __GLIBC_PREREQ
	wi->threadID = (int)syscall(SYS_gettid);
#else
	wi->threadID = (((int)pthread_self())>>12) & 0xFFFFF;
#endif
}

static void processWarnInfo(struct stlLogInfoStruct *WI)
{
	static stlMutex mux =stlMutexInitializer;	// For writing to file

	if ((WI == NULL) || (WI->sMsg == NULL)) return;

	if (printfHandler)
		printfHandler(WI->sMsg);
	if (printfHandler2)
		printfHandler2(WI);

	if ((WI->sMsg) && (WI->sMsg->iLen > 0))		// als printf handler lengte naar 0 heeft gezet doen we niets.
	{
		if (is_daemon){
			if (showMsTimes)
				syslog(LOG_WARNING,"[%05X]: ;%u; %s", WI->threadID, WI->msTime, WI->sMsg->sBuf);
			else
				syslog(LOG_WARNING,"[%05X]: %s", WI->threadID, WI->sMsg->sBuf);
		}else{
			int iLF = stlCount(WI->sMsg,10);
			if (showMsTimes)
			{
				if (iLF) fprintf(stdout,"[%4d]:;%u; %s"  ,WI->threadID,WI->msTime,WI->sMsg->sBuf);
				else     fprintf(stdout,"[%4d]:;%u; %s\n",WI->threadID,WI->msTime,WI->sMsg->sBuf);
			}else
			{
				if (iLF) fprintf(stdout,"[%4d]: %s"  ,WI->threadID,WI->sMsg->sBuf);
				else     fprintf(stdout,"[%4d]: %s\n",WI->threadID,WI->sMsg->sBuf);
			}
			fflush(stdout);
		}
	}

	if (msgFn[0]){
		int i;
		FILE *f1=NULL;
		STP Tm1 = NULL;
		if (WI->msTime)
			Tm1 = stlSetStf("\n%02d-%02d %02d:%02d:%02d;%07d;%s[%s]; ",WI->datM,WI->datD,WI->timH,WI->timM,WI->timS,WI->msTime,appFn,WI->sNam);
		else
			Tm1=stlSetStf("\n%04d-%02d-%02d %02d:%02d:%02d %s[%s]: ",WI->datY,WI->datM,WI->datD,WI->timH,WI->timM,WI->timS,appFn,WI->sNam);
		// Voeg aan messages bestand toe
		for (i=0;i<5;i++){
			stlMutexLock(&mux);
			f1=fopen(msgFn,"ab");
			if (f1) break;
			stlMutexUnlock(&mux);
			stlMsWait(10);
		}
		if (f1){
			int nri=stlCountChr(WI->sMsg->sBuf,_D1)+1;
			for (i=1;i<=nri;i++){
				STP Tm2=stlGetStr(WI->sMsg,i,0,0);
				if (Tm2->iLen){
					fwrite(Tm1->sBuf,1,Tm1->iLen,f1);
					fwrite(Tm2->sBuf,1,Tm2->iLen,f1);
				}
				stlFree(Tm2);
			}
			i=ftell(f1);	// Grootte van het bestand.
			fclose(f1);
			if (i > printfLogRotSize){	// Meer dan 4 Mb (backup naar .old)
				STP Tm2 = NULL;
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
	WI->sMsg = NULL;
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
void stlPrintfSetMsTimes(int iOn)
{
	showMsTimes = iOn;
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



#define _MaxWarnHist_	3000
static struct stlLogInfoStruct warnDb[_MaxWarnHist_];
static stlMutex warnMux=stlMutexInitializer;
static int  warnHead = 0;
static int  warnTail = 0;
static int  stlPrintThreadFlg=0;
int printfDroppedMsgs = 0;
static void _stlPrintThread(void *pUserData)
{
	struct stlLogInfoStruct WI;
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

		processWarnInfo(&WI);
	}
}
static int queueWarnGetFree()
{
	int iUsed = (warnHead - warnTail);
	if (iUsed < 0) iUsed += _MaxWarnHist_;
	return _MaxWarnHist_ - iUsed;
}


static void queueWarnInfo(struct stlLogInfoStruct *wi)
{
	stlMutexLock(&warnMux);
	if (queueWarnGetFree() > 10)
	{
		if (warnDb[warnHead].sMsg) stlFree(warnDb[warnHead].sMsg);
		memcpy(&warnDb[warnHead],wi,sizeof(struct stlLogInfoStruct));
		warnHead = (warnHead + 1) % _MaxWarnHist_;
	}else
	{
		printfDroppedMsgs++;
		stlFree(wi->sMsg);
	}
	stlMutexUnlock(&warnMux);
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

void stlDeamonize(void)
{
	int pid,fd;
	/* go into the background */
	if ((pid = fork()) < 0){
		syslog(LOG_ERR,"Could not fork into background: %m");
		exit(1);
	}
	/* parent process is finished */
	if (pid != 0) exit(0);
	setsid();
	umask(0);

	for (fd=0;fd<1024;fd++) close(fd);	// Close all handles including stdout en stdin

	//close(0); //STDIN
	//close(1); //STDOUT
	//close(2); //STDERR
	/* Reopen stdin descriptor on /dev/null */
	if ((fd = open("/dev/null", O_RDWR)) < 0) {   /* stdin */
		perror("open: /dev/null");
		return;
	}
	if (dup(fd) < 0) {				/* stdout */
		perror("dup: /dev/null");
		return;
	}
	if (dup(fd) < 0) {				/* stderr */
		perror("dup: /dev/null");
		return;
	}
	openlog(NULL,LOG_PID,LOG_DAEMON);
	is_daemon=1;
}

/* Overwrite library function of puts (printf is renamed to puts on compiletime if no params givven)
*/
int puts(const char *s)
{
	struct stlLogInfoStruct WI;
	initWarnInfo(&WI);
	WI.sMsg = stlSetSt(s);
	if (stlPrintfBackgroundActive)
		queueWarnInfo(&WI);
	else
		processWarnInfo(&WI);
	return 0;
}

#undef printf

int printf(const char *format, ...)
{
	va_list ap;
	struct stlLogInfoStruct WI;
	initWarnInfo(&WI);

	va_start(ap,format);
	WI.sMsg = stlSetSta(format,ap);
	va_end(ap);

	if (stlPrintfBackgroundActive)
		queueWarnInfo(&WI);
	else
		processWarnInfo(&WI);

	return 0;
}

/* Overwrite library function of printf so all normal output is send to syslog
*/
int stlPrintf(const char *format, ...)
{
	va_list ap;
	struct stlLogInfoStruct WI;
	initWarnInfo(&WI);

	va_start(ap,format);
	WI.sMsg = stlSetSta(format,ap);
	va_end(ap);

	if (stlPrintfBackgroundActive)
		queueWarnInfo(&WI);
	else
		processWarnInfo(&WI);

	return 0;
}

/* sets system wide lock so application can only be running once
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int setPidLock(const char *_LockFile_)
{
	FILE *f1;
	f1=fopen(_LockFile_,"w");
	if (f1){
		fprintf(f1,"%d\n",getpid());
		fclose(f1);
	}
	return 0;
}

/* Test of application already running
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int testPidLock(const char *_LockFile_)
{
	FILE *f1;
	char buf[50];
	int len,pid;

	f1=fopen(_LockFile_,"r");
	if (f1==NULL) return 0;

	len=fread(buf,1,sizeof(buf)-1,f1);
	fclose(f1);
	if (len>0){
		buf[len]=0;
		pid=atoi(buf);
		sprintf(buf,"/proc/%d/status",pid);
		f1=fopen(buf,"r");
		if (f1){fclose(f1);return 1;}	// proces bestaat dus geen lock mogelijk
	}
	return 0;
}
#endif
