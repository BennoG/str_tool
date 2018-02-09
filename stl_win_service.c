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
#include <winsvc.h>
#include <direct.h>

#include "stl_str.h"
//#include "stl_service.h"


DWORD (WINAPI *myGetLongPathNameA) (LPCSTR,LPSTR,DWORD)=NULL;
BOOL  (WINAPI *myChangeServiceConfig2A)(SC_HANDLE,DWORD,LPVOID)=NULL;
BOOL  (WINAPI *myQueryServiceConfig2A) (SC_HANDLE,DWORD,LPBYTE,DWORD,LPDWORD)=NULL;
BOOL  (WINAPI *myQueryServiceConfigA)  (SC_HANDLE,LPQUERY_SERVICE_CONFIGA,DWORD,LPDWORD)=NULL;

static void loadKernelFx()
{
	static int iDone=0;
	HINSTANCE hModule;
	if (iDone) return;
	iDone=1;
	hModule = GetModuleHandle("kernel32.dll");
	myGetLongPathNameA     =(void*) GetProcAddress(hModule,"GetLongPathNameA");
	hModule = GetModuleHandle("Advapi32.dll");
	myChangeServiceConfig2A=(void*) GetProcAddress(hModule,"ChangeServiceConfig2A");
	myQueryServiceConfig2A =(void*) GetProcAddress(hModule,"QueryServiceConfig2A");
	myQueryServiceConfigA  =(void*) GetProcAddress(hModule,"QueryServiceConfigA");
}

/* Deze functie moet op deze manier daar deze niet op alle platformen aanwezig is
*/
#undef RegisterServiceProcess
static DWORD RegisterServiceProcess(DWORD dwProcessId,DWORD dwType)
{
	typedef DWORD (WINAPI *PFUNCTION)(DWORD,DWORD);
	PFUNCTION  rsp;
	HINSTANCE   hK32;
	DWORD       Rc=0;

	hK32 = GetModuleHandle("kernel32.dll");
	if( hK32 != NULL ) {
		rsp = (PFUNCTION)GetProcAddress(hK32,"RegisterServiceProcess");
		if( rsp != NULL ) {
			Rc=rsp(dwProcessId,dwType);
			printf("RegisterServiceProcess %d",Rc);
		}
	}
	return Rc;
}


/* chdir naar directory en drive waar de huidige
** executable staat (argv[0]) als parameter mee
** geven.
** Gaat zelf drive letter en path aanvullen indien nodig.
** return: 0  Ok
**         -1 Error
*/
int stlSrvSetBasePath(char *sExeName)
{
	STP aExe;
	aExe=stlSrvGetFullExePath(sExeName);
	if (aExe==NULL) return -1;
	stlMakeLongName(aExe);
	stlDelDlm(aExe,-2,'\\');
	_chdrive(aExe->sBuf[0]&0x1F);
	_chdir(aExe->sBuf);
	stlFree(aExe);
	return 0;
}


/* Returns full drive:\path\EXE name of short EXE name (argv[0]) 
**  sExe  name of current EXE file (argv[0])
** Return NULL error
**        FullName
*/
STP stlSrvGetFullExePath(char *sExe)
{
	char sPath[150]="",*pPath,*pExt=".exe";
	int iDrv,iLen;					// 1=A  26=Z
	if (sExe==NULL) return NULL;
	if (strchr(sExe,':')==NULL){    // No drive letter given
		iDrv=_getdrive();
		pPath=sExe;
	}else{
		iDrv=sExe[0] & 0x1F;
		pPath=&sExe[2];
	}

	// check if name ends on .exe
	iLen=(int)strlen(pPath);
	if ((iLen>4)&&(stricmp(pPath+iLen-4,pExt)==0)) pExt="";

	if (pPath[0]!='\\'){			// No complete path given
		if (_getdcwd(iDrv,sPath,sizeof(sPath)-1)){
			return stlSetStf("%s\\%s%s",sPath,pPath,pExt);
		}
		return NULL;
	}
	return stlSetStf("%c:%s%s",iDrv+'A'-1,pPath,pExt);
}

/* Convert 8.3 path en filename in long filename
*/
void stlMakeLongName(STP aFn)
{
	STP aRes;
	if (aFn==NULL) return;
	if (aFn->iLen==0) return;
	aRes=stlInitLen(350,0);
	
	loadKernelFx();
	if ((myGetLongPathNameA)&&(myGetLongPathNameA(aFn->sBuf,aRes->sBuf,aRes->iLen))) stlExchange(aFn,aRes);

	stlFree(aRes);
}

/* Get version and / or description of current OS we are running on
**  piWinNT  (OUT) 0=win 95/98/me  1=NT/2k/XP/2k3
**  piMajor  (OUT) major version number
**  piMinor  (OUT) minor version number
**  pDescr   (OUT) description of current OS
** All parameters may be NULL if not needed
** Return 0 Ok
**       <0 Error
*/
int stlSrvGetOS(int *piWinNT,int *piMajor,int *piMinor,char *pDescr)
{
	OSVERSIONINFOEX osvi;

	if (piWinNT) *piWinNT=-1;
	if (piMajor) *piMajor=0;
	if (piMinor) *piMinor=0;
	if (pDescr ) pDescr[0]=0;

	memset(&osvi,0,sizeof(osvi));
	osvi.dwOSVersionInfoSize=sizeof(osvi);
	if (GetVersionEx((OSVERSIONINFO*)&osvi)){
	}else{
		stlMsWait(10);
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (GetVersionEx((OSVERSIONINFO*)&osvi)==0) return -1;
	}
	// "pv 1 v 4.10 A (Build 2222)"                Win98SE   (clean install)
	// "pv 1 v 4.10 A (Build 2222)"                Win98SE   (SP1)
	// "pv 2 v 4.0 Service Pack 6 (Build 1381)"    WinXP NT  (SP6)
	// "pv 2 v 5.1 Service Pack 1 (Build 2600)"    WinXP SP1
	// "pv 2 v 5.1 Service Pack 2 (Build 2600)"    WINXP SP2
	// "pv 2 v 5.2  (Build 3790)                   Win2k3
	printf ("pv %d v %d.%d %s (Build %d)",osvi.dwPlatformId,osvi.dwMajorVersion,osvi.dwMinorVersion,osvi.szCSDVersion,osvi.dwBuildNumber & 0xFFFF);
	if (pDescr) strcpy(pDescr,osvi.szCSDVersion);

	switch (osvi.dwPlatformId){
	case VER_PLATFORM_WIN32_NT:
		if (piWinNT) *piWinNT=1;
		if (piMajor) *piMajor=osvi.dwMajorVersion;
		if (piMinor) *piMinor=osvi.dwMinorVersion;
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if (piWinNT) *piWinNT=0;
		if (piMajor) *piMajor=osvi.dwMajorVersion;
		if (piMinor) *piMinor=osvi.dwMinorVersion;
		break;
	}
	return 0;
}




/* Install service for Windows
**  sExeName  Full name of program (including drive and path)
**  sSrvName  Name on witch the service is referred
**  sSrvDescr Short description of service
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager open error
**          2 Service manager add error (exists already ???)
*/
int stlSrvInstall(char *sExeName,char *sSrvName,char *sSrvDescr)
{
	int iPlat=0,iMajor=0;
	unsigned char* key95  = "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";
	unsigned char* keyNT4 = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	unsigned char* keyReg = NULL;

	SERVICE_DESCRIPTION SD;
	stlSrvGetOS(&iPlat,&iMajor,NULL,NULL);
	if ((iPlat==0)||(iMajor<=4)){					// Windows 95 / 98 / ME (NT)
		int iMin;
		char sKey[200];
		HKEY hKey;
		if (iPlat==0) keyReg=key95; else keyReg=keyNT4;
		//if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\RunServices",0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS){
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,keyReg,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS){
			iMin=sizeof(sKey)-1;
			if (RegQueryValueEx(hKey,TEXT(sSrvName),NULL,NULL,sKey,&iMin) == ERROR_SUCCESS){
				// Exists check contents
				if (stricmp(sExeName,sKey)){	// Wrong contents update new value
					RegSetValueEx(hKey,TEXT(sSrvName),0,REG_SZ,sExeName,(DWORD)strlen(sExeName));
				}
			}else{
				RegSetValueEx(hKey,TEXT(sSrvName),0,REG_SZ,sExeName,(DWORD)strlen(sExeName));
			}
			RegCloseKey(hKey);
			return 0;
		}
		return 1;
	}
	if (iPlat==1){					// Windows NT / XP
		SC_HANDLE amsSrvHdl,scm;
		scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
		if(!scm) return 1;
		amsSrvHdl=CreateService(
			scm,
			sSrvName,
			sSrvName,
			SERVICE_ALL_ACCESS,SERVICE_WIN32_OWN_PROCESS,/*SERVICE_DEMAND_START*/ SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			sExeName,
			0,0,0,0,0);
		if(!amsSrvHdl){
			CloseServiceHandle(scm);
			return 2;
		}
		memset(&SD,0,sizeof(SD));
		SD.lpDescription = sSrvDescr;
		
		loadKernelFx();
		myChangeServiceConfig2A(amsSrvHdl,SERVICE_CONFIG_DESCRIPTION,&SD);
		CloseServiceHandle(amsSrvHdl);
		CloseServiceHandle(scm);
		return 0;
	}
	return -1;
}
/* Install service for Windows
**  argv0     naam van .exe zoals in argv[0] gegeven
**  sSrvName  Name on witch the service is referred
**  sSrvDescr Short description of service
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager open error
**          2 Service manager add error (exists already ???)
*/
int stlSrvInstall2(char *argv0,char *sSrvName,char *sDescription)
{
	int iRes=0;
	STP aExe,aApp=NULL;
	aExe=stlSrvGetFullExePath(argv0);
	stlMakeLongName(aExe);
	if (sDescription==NULL) sDescription="";
	if ((sSrvName==NULL)||(sSrvName[0]==0))	// naam van de EXE gebruiken als service naam niet word gegeven
	{
		aApp=stlGetDlm(aExe,-2,'\\');
		sSrvName=aApp->sBuf;
	}
	if (aExe) iRes=stlSrvInstall(aExe->sBuf,sSrvName,sDescription); 
	else     {iRes=-1;aExe=stlSetSt("Unknown");}
	if (iRes==0) printf("service %s geregistreerd\n",aExe->sBuf);
	else         printf("Error service registratie %s\n",aExe->sBuf);
	if (iRes==0) stlSrvAutoRestart(sSrvName,1);
	stlFree(aExe);
	stlFree(aApp);
	return iRes;
}



/* Remove service for Windows
**  sSrvName  Name on witch the service is referred
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager openen error
*/
int stlSrvRemove(char *sSrvName)
{
	int iPlat=0,iMajor=0;
	unsigned char* key95  = "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";
	unsigned char* keyNT4 = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	unsigned char* keyReg = NULL;
	
	stlSrvGetOS(&iPlat,&iMajor,NULL,NULL);

	if ((iPlat==0)||(iMajor<=4)){					// Windows 95 / 98 / ME (NT4)
		HKEY hKey;
		if (iPlat==0) keyReg=key95; else keyReg=keyNT4;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,keyReg,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS){
			RegDeleteValue(hKey,TEXT(sSrvName));
			RegCloseKey(hKey);
			return 0;
		}
		return 1;
	}

	if (iPlat==1){					// Windows NT / XP
		int iRet;
		SC_HANDLE amsSrvHdl,scm;
		scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
		if(!scm) return 1;
		amsSrvHdl=OpenService(scm,sSrvName,SERVICE_ALL_ACCESS);
		if(!amsSrvHdl){CloseServiceHandle(scm);return 2;}
		iRet=DeleteService(amsSrvHdl);
		CloseServiceHandle(amsSrvHdl);
		CloseServiceHandle(scm);
		return 0;
	}
	return -1;
}


#define _BufSize_	4096
/* Property service auto restart aan of uit zetten
** Param:
** sSrvName  Naam waaronder service is geregistreerd
** iRestart  1 Auto restart 0 disable auto restart
** Return:  0 Ok
**          1 Kan service manager niet openen
**          2 Kan service niet openen (service bestaat niet)
**          3 Kan config van service niet opvragen
**          4 Out of memory
*/
int stlSrvAutoRestart(char *sSrvName,int iRestart)
{
	int iErr=0;
	LPQUERY_SERVICE_CONFIG    scConf =NULL;
	LPSERVICE_DESCRIPTION     scDescr=NULL;
	LPSERVICE_FAILURE_ACTIONS scFail =NULL;
	DWORD dwBytesNeeded;
	SC_HANDLE amsSrvHdl=NULL,scm=NULL;

	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if(scm==NULL) {iErr=1;goto cleanup;}
	amsSrvHdl=OpenService(scm,sSrvName,SERVICE_ALL_ACCESS);
	if(amsSrvHdl==NULL){iErr=2;goto cleanup;}

	scConf =LocalAlloc(LPTR,_BufSize_);
	scDescr=LocalAlloc(LPTR,_BufSize_);
	scFail =LocalAlloc(LPTR,_BufSize_);
	if ((scConf==NULL)||(scDescr==NULL)||(scFail==NULL)){iErr=4;goto cleanup;}

	// Get the configuration information. 
	loadKernelFx();
	if (!myQueryServiceConfigA (amsSrvHdl,scConf,_BufSize_,&dwBytesNeeded)){iErr=3;goto cleanup;}
	if (!myQueryServiceConfig2A(amsSrvHdl,SERVICE_CONFIG_DESCRIPTION,(void*)scDescr,_BufSize_,&dwBytesNeeded)){iErr=3;goto cleanup;}
	if (!myQueryServiceConfig2A(amsSrvHdl,SERVICE_CONFIG_FAILURE_ACTIONS,(void*)scFail,_BufSize_,&dwBytesNeeded)){iErr=3;goto cleanup;}

	if (iRestart){			// Enable auto-restart
		if (scFail->cActions==0){
			SC_ACTION sAct={SC_ACTION_RESTART,60000};
			scFail->cActions=1;
			scFail->lpsaActions=&sAct;
			myChangeServiceConfig2A(amsSrvHdl,SERVICE_CONFIG_FAILURE_ACTIONS,(void*)scFail);
		}
	}else{					// Disable auto-restart
		if (scFail->cActions){
			scFail->cActions=0;
			myChangeServiceConfig2A(amsSrvHdl,SERVICE_CONFIG_FAILURE_ACTIONS,(void*)scFail);
		}
	}

cleanup:
	if (scConf ) LocalFree(scConf );
	if (scDescr) LocalFree(scDescr);
	if (scFail ) LocalFree(scFail );
	if (amsSrvHdl) CloseServiceHandle(amsSrvHdl);
	if (scm)       CloseServiceHandle(scm);
	return iErr;
}
/* start of stop een service onder windows
**  sSrvName  Naar waaronder de service word gerefereerd
**  iStart    1=Start 0=Stop
** Return:  0 ok
**          1 Kan service manager niet openen
**          2 Kan service niet openen (service bestaat niet)
*/
int stlSrvStartStop(char *sSrvName,int iStart)
{
	SC_HANDLE amsSrvHdl,scm;
	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if(!scm) return 1;
	amsSrvHdl=OpenService(scm,sSrvName,SERVICE_ALL_ACCESS);
	if(!amsSrvHdl){
		CloseServiceHandle(scm);
		return 2;
	}
	if (iStart){
		StartService(amsSrvHdl,0,NULL);
	}else{
		SERVICE_STATUS m_SERVICE_STATUS;
		ControlService(amsSrvHdl,SERVICE_CONTROL_STOP,&m_SERVICE_STATUS);
	}
	CloseServiceHandle(amsSrvHdl);
	CloseServiceHandle(scm);
	return 0;
}

#if (WINVER >= 0x500)
/* Test if service is running
**  sSrvName  Naar waaronder de service word gerefereerd
** Return 1 Running
**        0 Not running
**       -1 Kan service manager niet openen
**       -2 Service bestaat niet of geen rechten
**       -3 Kan service status niet 
*/
int stlSrvRunning(char *sSrvName)
{
	DWORD dwNeed,dwRes;
	SERVICE_STATUS_PROCESS SP;
	SC_HANDLE hMgr,hSrv;
	hMgr=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if (hMgr==NULL) return -1;
	hSrv=OpenService(hMgr,sSrvName,SERVICE_ALL_ACCESS);
	if (hSrv==NULL)
	{
		CloseServiceHandle(hMgr);
		return -2;
	}
	dwRes=QueryServiceStatusEx(hSrv,SC_STATUS_PROCESS_INFO,(LPBYTE)&SP,sizeof(SP),&dwNeed);
	CloseServiceHandle(hSrv);
	CloseServiceHandle(hMgr);
	if (dwRes==0) return -3;
	if (SP.dwCurrentState==SERVICE_RUNNING) return 1;
	return 0;
}
#endif



static void ServiceMain(DWORD argc, LPTSTR *argv);
static void KillService(void);
DWORD ServiceExecutionThread(LPDWORD param);
void ServiceCtrlHandler(DWORD nControlCode);

static DWORD nServiceCurrentStatus=0;
static SERVICE_STATUS_HANDLE nServiceStatusHandle; 
static BOOL nServiceRunning=FALSE;
static HANDLE killServiceEvent=NULL;
static HANDLE hServiceThread  =NULL;
static char sServiceName[100]="";
static void (*mainFunction)()=NULL;


/* main service loop onder windows (keert pas terug als service word beeindigd)
** Param:
** sSrvName  Naar waaronder de service word gereffereerd
** fmain     main function to call
** Return:  0 ok
**          1 Kan service manager niet openen
**          2 Kan service niet openen (service bestaat niet)
*/
int stlSrvInitiate(char *sSrvName,void (*main)())
{
	SERVICE_TABLE_ENTRY servicetable[]={
		{sSrvName,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL,NULL}
	};
	BOOL success;
	int iPlat=0,iMajor=0;

	if (main==NULL) return 1;			 // no main funct givven
	mainFunction=main;

	stlSrvGetOS(&iPlat,&iMajor,NULL,NULL);
	if ((iPlat==0)||(iMajor<=4)){		// Windows 95/98/Me (NT4) (altijd service)
		RegisterServiceProcess(0,1);
		stlDeamonize();
		mainFunction();
		return 0;
	}

	memset (sServiceName,0,sizeof(sServiceName));
	strncpy(sServiceName,sSrvName,sizeof(sServiceName)-1);

	success=StartServiceCtrlDispatcher(servicetable);
	printf("StartServiceCtrlDispatcher %d",success);
	if(!success) return 1;	//error occured
	return 0;
}


static BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
								DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
								DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS nServiceStatus;
	nServiceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	nServiceStatus.dwCurrentState=dwCurrentState;
	if(dwCurrentState==SERVICE_START_PENDING){
		nServiceStatus.dwControlsAccepted=0;
	}else{
		nServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	}
	if(dwServiceSpecificExitCode==0){
		nServiceStatus.dwWin32ExitCode=dwWin32ExitCode;
	}else{
		nServiceStatus.dwWin32ExitCode=ERROR_SERVICE_SPECIFIC_ERROR;
	}
	nServiceStatus.dwServiceSpecificExitCode=dwServiceSpecificExitCode;
	nServiceStatus.dwCheckPoint=dwCheckPoint;
	nServiceStatus.dwWaitHint=dwWaitHint;

	success=SetServiceStatus(nServiceStatusHandle,&nServiceStatus);
	if(!success) KillService();	// kan loop veroorzaken

	return success;
}

static void KillService(void)
{
	nServiceRunning=FALSE;
	SetEvent(killServiceEvent);
	UpdateServiceStatus(SERVICE_STOPPED,NO_ERROR,0,0,0);
}


static BOOL StartServiceThread()
{	
	DWORD id;
	hServiceThread=CreateThread(0,0,
		(LPTHREAD_START_ROUTINE)ServiceExecutionThread,
		0,0,&id);
	if(hServiceThread==0){
		return FALSE;
	}else{
		nServiceRunning=TRUE;
		return TRUE;
	}
}

static void ServiceMain(DWORD argc, LPTSTR *argv)
{
	BOOL success;
	nServiceStatusHandle=RegisterServiceCtrlHandler(sServiceName,(LPHANDLER_FUNCTION)ServiceCtrlHandler);
	if(!nServiceStatusHandle) return;

	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,1,3000);
	if(!success) return;

	killServiceEvent=CreateEvent(0,TRUE,FALSE,0);
	if(killServiceEvent==NULL) return;

	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,2,1000);
	if(!success) return;

	success=StartServiceThread();
	if(!success) return;

	nServiceCurrentStatus=SERVICE_RUNNING;
	success=UpdateServiceStatus(SERVICE_RUNNING,NO_ERROR,0,0,0);
	if(!success) return;

	WaitForSingleObject(killServiceEvent,INFINITE);
	CloseHandle(killServiceEvent);
}





void ServiceCtrlHandler(DWORD nControlCode)
{
	BOOL success;
	switch(nControlCode){	
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		nServiceCurrentStatus=SERVICE_STOP_PENDING;
		success=UpdateServiceStatus(nServiceCurrentStatus,NO_ERROR,0,1,3000);
		KillService();
		return;
	default:
		break;
	}
	UpdateServiceStatus(nServiceCurrentStatus,NO_ERROR,0,0,0);
}

DWORD ServiceExecutionThread(LPDWORD param)
{
	mainFunction();
	return 0;
}



/*++
Routine Description:
This routine returns if the service specified is running interactively
(not invoked \by the service controller).

Arguments:
None

Return Value:
BOOL - TRUE if we are a service
FALSE if we are a normal EXE.

Note:
--*/
BOOL WINAPI stlSrvIsService( VOID )
{
	HANDLE hProcessToken = NULL;
	DWORD groupLength = 50;

	PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

	SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
	PSID InteractiveSid = NULL;
	PSID ServiceSid = NULL;
	DWORD i;

	// Start with assumption that process is an EXE, not a Service.
	BOOL bSrv = FALSE;

	int iPlat=0,iMajor=0;
	stlSrvGetOS(&iPlat,&iMajor,NULL,NULL);
	if ((iPlat==0)||(iMajor<=4)){bSrv=TRUE;goto ret;}		// Windows 95/98/Me (NT4) (altijd service)

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,&hProcessToken)) goto ret;

	if (groupInfo == NULL)  goto ret;

	if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,groupLength, &groupLength)){
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto ret;

		LocalFree(groupInfo);
		groupInfo = NULL;

		groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

		if (groupInfo == NULL)
			goto ret;

		if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,groupLength, &groupLength)){
			goto ret;
		}
	}

	//
	//  We now know the groups associated with this token.  We want to look to see if
	//  the interactive group is active in the token, and if so, we know that
	//  this is an interactive process.
	//
	//  We also look for the "service" SID, and if it's present, we know we're a service.
	//
	//  The service SID will be present iff the service is running in a
	//  user account (and was invoked by the service controller).
	//

	if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, &InteractiveSid)){
		goto ret;
	}

	if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0, 0, 0, 0, 0, &ServiceSid)){
		goto ret;
	}

	for (i = 0; i < groupInfo->GroupCount ; i += 1) {
		SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
		PSID Sid = sanda.Sid;

		//
		//  Check to see if the group we're looking at is one of
		//  the 2 groups we're interested in.
		//
		if (EqualSid(Sid, InteractiveSid)){
			//
			//  This process has the Interactive SID in its
			//  token.  This means that the process is running as
			//  an EXE.
			//
			goto ret;
		}else if (EqualSid(Sid, ServiceSid)){
			//
			//  This process has the Service SID in its
			//  token.  This means that the process is running as
			//  a service running in a user account.
			//
			bSrv = TRUE;
			goto ret;
		}
	}

	//
	//  Neither Interactive or Service was present in the current users token,
	//  This implies that the process is running as a service, most likely
	//  running as LocalSystem.
	//
	bSrv = TRUE;

ret:

	if (InteractiveSid) FreeSid(InteractiveSid);
	if (ServiceSid)     FreeSid(ServiceSid);
	if (groupInfo)      LocalFree(groupInfo);
	if (hProcessToken)  CloseHandle(hProcessToken);
	return(bSrv);
}

#endif // _WIN32

