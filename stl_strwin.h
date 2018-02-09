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

#ifndef _STL_STR_WIN_H_
#define _STL_STR_WIN_H_

#include <windows.h>
#include <stdio.h>
#include <string.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* Getopt functions (like linux getopt)
 */
extern char	*optarg;	// global argument pointer
extern int	 optind; 	// global argv index
int getopt(int argc, char *argv[], char *optstring);

# define atoi_b(a,b) strtol(a,NULL,b);

/* syslog function emulation on windows
 */
void syslog(int iCode,char *fmt,...);
# define LOG_INFO			1
# define LOG_DEBUG		2
# define LOG_ERROR		3


struct stlLogInfoStruct
{
	STP sMsg;				// hier komt melding in te staan
	char sNam[50];			// naam van de thread
	int timH,timM,timS;		// huidige tijd
	int datY,datM,datD;		// huidige datum
	unsigned int msTime;	// bericht tijd in ms
	int threadID;			// thread ID
};

/* Set user handler for printf output of application
**  userHandler  function to be called on each printf
*/
void stlPrintfHandler(void (*userHandler)(STP));
void stlPrintfHandler2(void (*userHandler)(struct stlLogInfoStruct*));


// printf redirection in windows
void stlPrintfSetMsTimes(int iOn);
void stlPrintfThread();				// do printing in separate thread. no time delay on printf commands
void stlPrintfSetFile(FILE *fOut);
void stlPrintfSetHdlg(HWND hWnd,int iLstID);
void stlPrintfSetLogFn(char *sLogFn);
void stlPrintfSetAppName(char *sName);
int  stlPrintf(const char *format , ... );

extern int printfLogRotCount;	// default rotate to .old if count >= 0 rotate from count up never cleanup
extern int printfLogRotSize;	// default 4M size of log before rotated

#ifdef printf
#  undef printf
#endif
#define printf stlPrintf

// MSVS4.2 == 1020  (vstudio 4.2)
// MSVS5.0 == 1100  (vstudio 5.0)
// MSVS6.0 == 1200	(vstudio 6.0)
// MSVS7.0 == 1300  (vstudio .NET 2002)
// MSVS7.1 == 1310  (vstudio .NET 2003)
// MSVC8.0 == 1400  (vstudio .NET 2005)
#if  (_MSC_VER > 1310)
#  pragma warning( disable : 4996 )
#endif


/* Get value from registry
 *	 sKey   key to registry (full path)
 *   sName  Name of registry value
 * Return NULL error
 *        STP  contens of reg value
 */
STP stlGetRegVal(char *sKey,char *sName);

/* Delete key in registry
 *	sKey    key to delete
 *  bUncond Auto remove subkeys
 * return 0 ok
 *        -1 error
 */
int stlDelRegKey(char *sKey,BOOL bUncond);

/* Set value in registry (or delete value if pValue = NULL)
 *	sKey   Complete path to reg key
 *  sName  Name of value to set or delete
 *  pValue Value to write (NULL will delete value)
 *  dwType REG_DWORD REG_SZ REG_BINARY
 * return 0 ok
 *        -1 error
 */
int stlSetRegVal(char *sKey,char *sName,void *pValue,DWORD dwType);

/* Returns full drive:\path\EXE name of short EXE name (argv[0]) 
**  sExe  name of current EXE file (argv[0])
** Return NULL error
**        FullName
*/
STP stlSrvGetFullExePath(char *sExe);

/* chdir naar directory en drive waar de huidige
** executable staat (argv[0]) als parameter mee
** geven.
** Gaat zelf drive letter en path aanvullen indien nodig.
** return: 0  Ok
**         -1 Error
*/
int stlSrvSetBasePath(char *sExeName);

/* Convert 8.3 path en filename in long filename
*/
void stlMakeLongName(STP aFn);

/* Get version and / or description of current OS we are running on
**  piWinNT  (OUT) 0=win 95/98/me  1=NT/2k/XP/2k3
**  piMajor  (OUT) major version number
**  piMinor  (OUT) minor version number
**  pDescr   (OUT) description of current OS
** All parameters may be NULL if not needed
** Return 0 Ok
**       <0 Error
*/
int stlSrvGetOS(int *piWinNT,int *piMajor,int *piMinor,char *pDescr);

/* Install service for Windows
**  sExeName  Full name of program (including drive and path)
**  sSrvName  Name on witch the service is referred
**  sSrvDescr Short description of service
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager open error
**          2 Service manager add error (exists already ???)
*/
int stlSrvInstall(char *sExeName,char *sSrvName,char *sSrvDescr);

/* Install service for Windows
**  argv0     naam van .exe zoals in argv[0] gegeven
**  sSrvName  Name on witch the service is referred
**  sSrvDescr Short description of service
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager open error
**          2 Service manager add error (exists already ???)
*/
int stlSrvInstall2(char *argv0,char *sSrvName,char *sDescription);

/* Remove service for Windows
**  sSrvName  Name on witch the service is referred
** Return: -1 unsupported OS 
**          0 ok
**          1 Service manager openen error
*/
int stlSrvRemove(char *sSrvName);

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
int stlSrvAutoRestart(char *sSrvName,int iRestart);

/* start of stop een service onder windows
**  sSrvName  Naar waaronder de service word gerefereerd
**  iStart    1=Start 0=Stop
** Return:  0 ok
**          1 Kan service manager niet openen
**          2 Kan service niet openen (service bestaat niet)
*/
int stlSrvStartStop(char *sSrvName,int iStart);

#if (WINVER >= 0x500)
/* Test if service is running
**  sSrvName  Naar waaronder de service word gerefereerd
** Return 1 Running
**        0 Not running
**       -1 Kan service manager niet openen
**       -2 Service bestaat niet of geen rechten
**       -3 Kan service status niet 
*/
int stlSrvRunning(char *sSrvName);
#endif

/* main service loop onder windows (keert pas terug als service word beëindigd)
** sSrvName  Naar waaronder de service word gerefereerd
** fmain     main function to call
** Return:  0 ok
**          1 Kan service manager niet openen
**          2 Kan service niet openen (service bestaat niet)
*/
int stlSrvInitiate(char *sSrvName,void (*main)());

/* This routine returns if the service specified is running interactively
** (not invoked \by the service controller).
** Return Value:
**   BOOL - TRUE if we are a service
**   FALSE if we are a normal EXE.
*/
BOOL WINAPI stlSrvIsService( VOID );


#ifdef  __cplusplus
}
#endif

#endif _STL_STR_WIN_H_

