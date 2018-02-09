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

#include "stl_str.h"
#include "stl_thread.h"

#ifdef __linux__
#  include <getopt.h>
#  include <unistd.h>
#  include <errno.h>
#  include <sys/wait.h>
#  ifndef HANDLE
#    define HANDLE int
#  endif
#  ifndef CloseHandle
#    define CloseHandle(a) close(a)
#  endif
#endif

typedef struct
{
	int iAbort,iRun,iWait;
	HANDLE rPipe,wPipe;
	void(*fxOut)(char *sData,int iLen,void *pUserData);
	void *pUserData;
}pipeThread;

static void pipeTask(void *p)
{
#ifdef _WIN32
	DWORD  reDword; 
	BOOL res;
#else
	int res,reDword;
#endif

	char   buf[256];
	pipeThread *pt=p;
	pt->iRun=1;
#ifdef _DEBUG
	printf("Enter pipe task");
#endif
	if (pt->wPipe) CloseHandle(pt->wPipe);
	do
	{
		if (pt->iAbort) break;
#ifdef _WIN32
		res=ReadFile(pt->rPipe,buf,sizeof(buf)-1,&reDword,0);
#else
		reDword=read(pt->rPipe,buf,sizeof(buf)-1);
		res=reDword>0?1:0;	// res 0 in case of error
#endif
		if (reDword>0)
		{
			buf[reDword]=0;
			if (pt->fxOut) pt->fxOut(buf,reDword,pt->pUserData);
#ifdef _DEBUG
			if (pt->fxOut == NULL) printf("(%s) %s\n",stlThreadName(),buf);
#endif
		}
	}while (res);
	CloseHandle(pt->rPipe);
#ifdef _DEBUG
	printf("exit pipe task");
#endif
	pt->iRun=0;
	if (pt->iWait==0) free(pt);	// no waiting is done so cleanup ourselves
}

#define _MaxArg_ 15

#ifdef __linux__
/* Spawn an other proces an redirect stdout and stderr to user definable functions.
**  sCommand   command to execute including parameters
**  iWait      (1)wait for the child proces to exit (return == exit value of child)  (0)return immediately
**  fxOut      function to be called when child writes to stdout (optional)
**  pUserOut   data pointer witch is passed to fxOut
**  fxErr      function to be called when child writes to stderr (optional)
**  pUserErr   data pointer witch is passed to fxErr
** Return -1   unable to create pipe
**        -2   unable to fork current proces
**       >=0   Child proces exit status (if iWait set) or 0 (if iWait = 0)
*/
int stlExecPipe(char *sCommand,int iWait,void(*fxOut)(char *sData,int iLen,void *pUserOut),void *pUserOut,void(*fxErr)(char *sData,int iLen,void *pUserErr),void *pUserErr)
{
	int i,nri,iPid,iRes=0,iFdPipe[2];
	pipeThread *PTO,*PTE;
	STP  aAr[_MaxArg_],sCmd;
	char *argv[_MaxArg_];

	//Create pipes to write and read data
	PTE=malloc(sizeof(pipeThread));
	PTO=malloc(sizeof(pipeThread));
	memset(PTE ,0 ,sizeof(pipeThread));
	memset(PTO ,0 ,sizeof(pipeThread));
	memset(aAr ,0 ,sizeof(aAr));
	if (pipe(iFdPipe)==-1){printf("Can not open pipe %d\n",errno); return -1;}
	PTO->rPipe=iFdPipe[0];
	PTO->wPipe=iFdPipe[1];
	PTO->fxOut    =fxOut;
	PTO->pUserData=pUserOut;
	PTO->iWait    =iWait;

	if (pipe(iFdPipe)==-1){printf("Can not open pipe %d\n",errno); return -1;}
	PTE->rPipe=iFdPipe[0];
	PTE->wPipe=iFdPipe[1];
	PTE->fxOut    =fxErr;
	PTE->pUserData=pUserErr;
	PTE->iWait    =iWait;

	iPid=fork();
	switch (iPid)
	{
	case -1:	// error kan niet splitsen
		printf("Kan child proces niet starten\n"); 
		return -2;
		break;
	case 0:		// Wij zijn het child proces.
		dup2(PTO->wPipe,1);	// stdout aan pipe knopen
		dup2(PTE->wPipe,2); // stderr aan pipe knopen
		for (i=3;i<1024;i++) close(i);  // Close all files except stdin(0), stdout(1) en stderr(2)
		close(PTO->rPipe);close(PTO->wPipe);close(PTE->rPipe);close(PTE->wPipe);// zou niet hoeven daar de for next dit doet
		setsid();
		sCmd=stlSetSt(sCommand);
		nri=stlCount(sCmd,' ')+1;
		if (nri>=_MaxArg_) nri=_MaxArg_-1;
		for (i=1;i<=nri;i++) aAr[i-1]=stlGetDlm(sCmd,i,' ');
		for (i=0;i<_MaxArg_;i++)
		{
			if (aAr[i]) argv[i]=aAr[i]->sBuf;
			else        argv[i]=NULL;
		}
		execvp(argv[0],argv);	// never exets
		//printf("error execvp"); // geen printf hier dat corrupt memory om de een of andere reden
		exit(99);
		break;
	default:	// Ok wij zijn het aanroepend (master) process
		stlThreadStart(pipeTask,1,PTO,"pipe Out");
		stlThreadStart(pipeTask,1,PTE,"pipe Err");
		if (iWait){
			waitpid(iPid,&iRes,0);
			if ((iRes>0) && ((iRes & 0x00FF)==0)) iRes>>=8;	// return code van child proces weer herstellen
			PTO->iAbort=PTE->iAbort=1;
			while ((PTO->iRun) || (PTE->iRun)) stlMsWait(1);
			free(PTO);
			free(PTE);
		}
		break;
	}
	return iRes;
}
#endif

#ifdef _WIN32

static int DisplayError(int iRet,char *pszAPI)
{
	LPVOID lpvMessageBuffer;
	CHAR szPrintBuffer[512];

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	wsprintf(
		szPrintBuffer,
		"ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
		pszAPI, GetLastError(), (char *)lpvMessageBuffer);

	printf("%s",szPrintBuffer);
	LocalFree(lpvMessageBuffer);
	return iRet;
}

static STP stlFixSpawnApp(char *sApp)
{
	OSVERSIONINFO osvi;
	STP Tm1,Tm2;

	Tm1=stlSetSt(sApp);
	stlConvert(Tm1,'.',_D1);
	stlConvert(Tm1,'\"',' ');
	Tm2=stlGetStr(Tm1,-2,0,0);
	stlStrip(Tm2);
	stlFree(Tm1);
	if ((stricmp(Tm2->sBuf,"BAT")==0)||(stricmp(Tm2->sBuf,"CMD")==0)) {        // Het is een batch file
		memset(&osvi,0,sizeof(osvi));
		osvi.dwOSVersionInfoSize=sizeof(osvi);
		GetVersionEx(&osvi);
		if (osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
			Tm1=stlSetStf("cmd.exe /E:4096 /C %s",sApp);
		else
			Tm1=stlSetStf("command.com /E:2048 /C %s",sApp);

		printf("Fixed spawn ==>%s<== to ==>%s<==\n",sApp,Tm1->sBuf);
	}else{
		Tm1=stlSetSt(sApp);
	}
	stlFree(Tm2);
	return Tm1;
}


/* Spawn an other proces an redirect stdout and stderr to user definable functions.
**  sCommand   command to execute including parameters
**  iWait      (1)wait for the child proces to exit (return == exit value of child)  (0)return immediately
**  fxOut      function to be called when child writes to stdout (optional)
**  pUserOut   data pointer witch is passed to fxOut
**  fxErr      function to be called when child writes to stderr (optional)
**  pUserErr   data pointer witch is passed to fxErr
** Return -1   unable to create pipe
**        -2   unable to fork current proces
**       >=0   Child proces exit status (if iWait set) or 0 (if iWait = 0)
*/
int stlExecPipe(char *sCommand,int iWait,void(*fxOut)(char *sData,int iLen,void *pUserOut),void *pUserOut,void(*fxErr)(char *sData,int iLen,void *pUserErr),void *pUserErr)
{
	STP sCmd;
	pipeThread *PTO,*PTE;
	int    iRes;
	SECURITY_ATTRIBUTES secattr; 
	STARTUPINFO         sInfo; 
	PROCESS_INFORMATION pInfo; 

	//Create pipes to write and read data
	PTE=malloc(sizeof(pipeThread));
	PTO=malloc(sizeof(pipeThread));
	ZeroMemory(PTE    ,sizeof(pipeThread));
	ZeroMemory(PTO    ,sizeof(pipeThread));
	ZeroMemory(&secattr,sizeof(secattr));
	secattr.nLength        = sizeof(secattr);
	secattr.bInheritHandle = TRUE;
	if (CreatePipe(&PTO->rPipe,&PTO->wPipe,&secattr,0)==0) return -1;
	if (CreatePipe(&PTE->rPipe,&PTE->wPipe,&secattr,0)==0) return -1;
	PTO->fxOut    =fxOut;
	PTO->pUserData=pUserOut;
	PTO->iWait    =iWait;
	PTE->fxOut    =fxErr;
	PTE->pUserData=pUserErr;
	PTE->iWait    =iWait;

	ZeroMemory(&sInfo,sizeof(sInfo));
	ZeroMemory(&pInfo,sizeof(pInfo));
	sInfo.cb        =sizeof(sInfo);
	sInfo.dwFlags   =STARTF_USESTDHANDLES;
	sInfo.hStdInput =NULL; 
	sInfo.hStdOutput=PTO->wPipe; 
	sInfo.hStdError =PTE->wPipe;

	//Create the process here.
	sCmd=stlFixSpawnApp(sCommand);
	//iRes=CreateProcess(NULL,sCmd->sBuf,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,GetEnvironmentStrings(),0,&sInfo,&pInfo);
	iRes=CreateProcess(NULL,sCmd->sBuf,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,NULL,0,&sInfo,&pInfo);
	stlFree(sCmd);
	if (!iRes){
		DisplayError(1,"CreateProcess");
		CloseHandle(PTO->rPipe);
		CloseHandle(PTO->wPipe);
		CloseHandle(PTE->rPipe);
		CloseHandle(PTE->wPipe);
		free(PTO);
		free(PTE);
		return -2;
	}
	stlThreadStart(pipeTask,1,PTO,"pipe Out");
	stlThreadStart(pipeTask,1,PTE,"pipe Err");

	if (iWait){
		int i;
		// Wait until child process exits.
		WaitForSingleObject(pInfo.hProcess,INFINITE);
		GetExitCodeProcess (pInfo.hProcess,&iRes);
		// blijf max 1 sec wachten op netjes afronden van pipe data stromen.
		for (i = 0; i < 1000; i++)
		{
			stlMsWait(1);
			if ((PTO->iRun==0) && (PTE->iRun==0)) break;	// beide pipe streams zijn gesloten.
		}
		// wacht totdat threads zijn gestopt (nodig omdat anders handles de mist in gaan)
		PTO->iAbort=PTE->iAbort=1;
		while ((PTO->iRun) || (PTE->iRun)) stlMsWait(1);
		free(PTO);
		free(PTE);
	}else iRes=0;

	// Close process and thread handles.
	CloseHandle(pInfo.hProcess);
	CloseHandle(pInfo.hThread );
	return iRes;
}

#endif


