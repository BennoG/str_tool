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
#include <ras.h>
#include "stl_str.h"
#include "stl_ras.h"

static HRASCONN m_ConnectionHandle=0;
static STP errStr=NULL;

#pragma comment(lib, "rasapi32.lib")

//[HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Services\RemoteAccess\Parameters\Ip\StaticAddressPool\0]
//"From"=dword:0a010200
//"To"=dword:0a010209


// TODO: Ras meldingen via fifo op kunnen halen

// 
#define _MaxFifo_	20
struct rasMsgFifo{
	int iHead;
	int iTail;
	STP sMsgs[_MaxFifo_];
};
static struct rasMsgFifo rasFifo;
static stlMutex          rasMux=stlMutexInitializer;
static void rasInitFifo(void)
{
	static int iDone=0;
	if (iDone==0){
		iDone=1;
		memset(&rasFifo,0,sizeof(rasFifo));
	}
}
/* Add message to ras message fifo
 * fmt formatted string like printf()
 */
static void rasMessage(char *fmt,...)
{
	int iHd,i;
	STP sMsg;
	va_list ap;
	rasInitFifo();
	va_start(ap,fmt);
	sMsg=stlSetSta(fmt,ap);
	va_end(ap);

	for (i=0;i<sMsg->iLen;i++){
		if (sMsg->sBuf[i]<32 ) sMsg->sBuf[i]=' ';
		if (sMsg->sBuf[i]>127) sMsg->sBuf[i]=' ';
	}

	printf("%s\n",sMsg->sBuf);
	stlMutexLock(&rasMux);
	iHd=rasFifo.iHead;
	if (++rasFifo.iHead >= _MaxFifo_) rasFifo.iHead=0;
	if (rasFifo.iHead == rasFifo.iTail){
		if (++rasFifo.iTail>= _MaxFifo_) rasFifo.iTail=0;
	}
	if (rasFifo.sMsgs[iHd]) 
		stlFree(rasFifo.sMsgs[iHd]);
	rasFifo.sMsgs[iHd]=sMsg;
	stlMutexUnlock(&rasMux);
}

/* Get message from ras message fifo
 *  iDestruct  !=0 remove message from fifo after reading
 * Return STP (message)
 *        NULL (no messages in fifo)
 */
static STP _stlRasGetFiFoMsg(int iDestruct)
{
	STP sRes;
	rasInitFifo();
	if (rasFifo.iHead == rasFifo.iTail) return NULL;
	stlMutexLock(&rasMux);
	if (iDestruct){
		sRes=rasFifo.sMsgs[rasFifo.iTail];
		rasFifo.sMsgs[rasFifo.iTail]=NULL;
		if (++rasFifo.iTail>=_MaxFifo_) rasFifo.iTail=0;
	}else{
		sRes=stlCopy(rasFifo.sMsgs[rasFifo.iTail]);
	}
	stlMutexUnlock(&rasMux);
	return sRes;

}

/* Get message from ras message fifo
 * Return STP (message)
 *        NULL (no messages in fifo)
 */
STP stlRasGetMsgFiFo(void)
{
	return _stlRasGetFiFoMsg(1);
}

/* Get message from ras message fifo (non destructive)
 * Return STP (message)
 *        NULL (no messages in fifo)
 */
STP stlRasPeekMsgFiFo(void)
{	
	return _stlRasGetFiFoMsg(0);
}

/* Create a new RAS entry
**  psEntName   name of RAS entry to create
** Return 0 Ok
**       -1 Out of memory
**       -2 Invalid entry name
**       -3 No valid modem found to use
**       -4 Error updating phone book
*/
static int stlRasCreateEntry(char *psEntName)
{
	RASENTRY *pRE;
	RASDEVINFO *pDI;
	DWORD dwSz,dwNr;
	int iRes,i;
	int iDevice;
	dwSz=dwNr=0;
	
	iRes=RasValidateEntryName(NULL,psEntName);
	if (iRes==ERROR_ALREADY_EXISTS) return 0;	// Exists no need to create
	if (iRes!=ERROR_SUCCESS) return -2;			// Unknown error
	
	// find all RAS devices in system
	RasEnumDevices(NULL, &dwSz, &dwNr);			// Ask number of bytes needed for all devices
	pDI = malloc(dwSz);
	if (pDI==NULL) return -1;					// out of memory
	memset(pDI,0,dwSz);
	pDI->dwSize = sizeof(RASDEVINFO);
	
	iRes = RasEnumDevices(pDI, &dwSz, &dwNr);	// Ask all dialing devices
	if (iRes != 0){
		rasMessage("can't find a valid modem=%d",iRes);
		free(pDI);
		return -3;
	}

	iRes=-1;			// Voorkeur device
	iDevice=0;			// Default first device to use
	for (i=0; i < (int)dwNr; i++) {
		rasMessage("%d %s %s",i,pDI[i].szDeviceType,pDI[i].szDeviceName);
		if (strcmp("modem",pDI[i].szDeviceType)==0){
			iDevice=i;	// we have found a modem device
			if (strstr(pDI[i].szDeviceName,"ppp"     )) iRes=i;	// most likely to be the dial-out modem
			if (strstr(pDI[i].szDeviceName,"PPP"     )) iRes=i;	// most likely to be the dial-out modem
			if (strstr(pDI[i].szDeviceName,"port 1"  )) iRes=i;	// most likely to be the dial-out modem
			if (strstr(pDI[i].szDeviceName,"Internet")) iRes=i;	// most likely to be the dial-out modem
		}
	}
	if (iRes>=0) iDevice=iRes;	// prefer most likely dial-out modem
	rasMessage("using %s %s",pDI[iDevice].szDeviceType,pDI[iDevice].szDeviceName);

	pRE=malloc(sizeof(RASENTRY));
	if (pRE==NULL){free(pDI); return -1;}		// out of memory
	memset(pRE,0,sizeof(RASENTRY));
	dwSz=pRE->dwSize=sizeof(RASENTRY);

	pRE->dwfOptions       =RASEO_UseCountryAndAreaCodes | RASEO_IpHeaderCompression | RASEO_SwCompression;
	pRE->dwfOptions      |=RASEO_RemoteDefaultGateway   | RASEO_ModemLights;
	pRE->dwCountryID      =31;
	pRE->dwCountryCode    =31;
	pRE->dwfNetProtocols  =RASNP_Ip;
	pRE->dwFramingProtocol=RASFP_Ppp;
	strcpy(pRE->szAreaCode,"06760");
	strcpy(pRE->szLocalPhoneNumber,"12321");
	strcpy(pRE->szDeviceType,pDI[iDevice].szDeviceType);
	strcpy(pRE->szDeviceName,pDI[iDevice].szDeviceName);
	iRes=RasSetEntryProperties(NULL,psEntName,pRE,dwSz,NULL,0);
	free(pDI);
	free(pRE);
	if (iRes) return -4;						// error updating phone book
	return 0;
}


static void stlRasExitHangup(void)
{
	if (m_ConnectionHandle){
		stlRasHangup();
	}
}
static void stlRasInit(void)
{
	static int done=0;
	if (done) return;
	done++;
	atexit(stlRasExitHangup);
}
/* Get status of RAS connection
**	   <1>  0=Ok -99=No info
**     <2>  2=Calling 5=Logging in 8192=Connected
**     <3>  600=Calling/logging in   0=Logged in
**     <4>  "modem" ....
**     <5>  "Descriptoin of modem"
**     <6,1> "My IP adres"
**     <6,2> "Remote IP adres"
**     <7,1> bytes transmitted
**     <7,2> bytes received
**     <7,3> frames transmitted
**     <7,4> frames received
*/
STP stlRasStatus(void)
{
	STP aRes;
	int iRes;
#if (WINVER >= 0x500)
	RAS_STATS     stats;
#endif
	RASCONNSTATUS status;
	RASPPPIP      pppip;
	stlRasInit();
	status.dwSize=sizeof(status);
	iRes=RasGetConnectStatus(m_ConnectionHandle,&status);
	if (iRes) return stlSetSt("-99");
	memset(&pppip,0,sizeof(pppip));
	pppip.dwSize=sizeof(pppip);
	RasGetProjectionInfo(m_ConnectionHandle,RASP_PppIp,&pppip,&pppip.dwSize);


	aRes=stlSetSt("0");
	stlStoreInt(status.rasconnstate ,aRes,2,0,0);// 2=bellen 5=inloggen   8192=ingelogd
	stlStoreInt(status.dwError      ,aRes,3,0,0);// 600=bellen / inloggen    0=ingelogd
	stlStoreStr(status.szDeviceType ,aRes,4,0,0);// modem
	stlStoreStr(status.szDeviceName ,aRes,5,0,0);// Lucent Win Modem
	stlStoreStr(pppip.szIpAddress   ,aRes,6,0,0);// IP adres van verbinding
	stlStoreStr(pppip.szServerIpAddress,aRes,6,2,0);// IP adres van verbinding

#if (WINVER >= 0x500)
	memset(&stats,0,sizeof(stats));
	stats.dwSize=sizeof(stats);
	RasGetConnectionStatistics(m_ConnectionHandle,&stats);
	stlStoreInt(stats.dwBytesXmited ,aRes,7,1,0);// bytes transmitted
	stlStoreInt(stats.dwBytesRcved  ,aRes,7,2,0);// bytes received
	stlStoreInt(stats.dwFramesXmited,aRes,7,3,0);// frames transmitted
	stlStoreInt(stats.dwFramesRcved ,aRes,7,4,0);// frames received
#endif

	return aRes;
}

int stlRasHangup(void)
{
	RASCONNSTATUS status;
	stlRasInit();
	RasHangUp(m_ConnectionHandle);
	status.dwSize=sizeof(status);
	while (RasGetConnectStatus(m_ConnectionHandle,&status) != ERROR_INVALID_HANDLE) Sleep(0);
	m_ConnectionHandle=0;
	return 0;
}

/* Create dial-out modem connection
**	sCfg configuration record
**       <1>   Phone book entry naam
**       <2>   Phone number
**       <3,1> User name
**       <3,2> User password
**       <4>   Domain name
**       <5>   My IP address
** Return 0 Ok
**       -1 Out of memory
**       -2 Invalid entry name
**       -3 No valid modem found to use
**       -4 Error updating phone book
**       -5 dial error (no connection, dial-tone, etc)
*/
int stlRasDial(STP sCfg)
{
	STP sTmp,sEntName;
	int errNr;
	RASDIALPARAMS dialParams;
	LPRASENTRY lpRasEntry;
	LPTSTR lpszEntry;
	DWORD  entSize;
	stlRasInit();

	rasMessage("Start RAS %s",sCfg->sBuf);

	sEntName=stlGetStr(sCfg,1,0,0);		// phone book entry name

	// Fix name of modem entry
	if ((sEntName)&&(sEntName->iLen))	lpszEntry=sEntName->sBuf;	else lpszEntry="stl dialout";

	// Create entry if not exists
	errNr=stlRasCreateEntry(lpszEntry);
	if (errNr){rasMessage("RasCreateEntry error number %d",errNr);stlFree(sEntName); return errNr;}

	// Set extra info for phone book entry 
	entSize=0;
	RasGetEntryProperties(NULL,lpszEntry,NULL,&entSize,NULL,NULL);	// memory needed for entry
	lpRasEntry=malloc(entSize);
	memset(lpRasEntry,0,entSize);
	lpRasEntry->dwSize=sizeof(RASENTRY);
	errNr=RasGetEntryProperties(NULL,lpszEntry,lpRasEntry,&entSize,NULL,NULL);
	if (errNr){rasMessage("RasGetEntryProperties error number %d",errNr);stlFree(sEntName); return errNr;}

	// My own ip address
	sTmp=stlGetStr(sCfg,5,0,0);
	if (sTmp->iLen){
		stlConvert(sTmp,'.',_D1);
		lpRasEntry->ipaddr.a=stlGetInt(sTmp,1,0,0);
		lpRasEntry->ipaddr.b=stlGetInt(sTmp,2,0,0);
		lpRasEntry->ipaddr.c=stlGetInt(sTmp,3,0,0);
		lpRasEntry->ipaddr.d=stlGetInt(sTmp,4,0,0);
		lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
	}else
	{
		lpRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;
	}
	stlFree(sTmp);
	errNr=RasSetEntryProperties(NULL,lpszEntry,lpRasEntry,entSize,NULL,0);
	free(lpRasEntry);

	// Start dialing phone-book entry
	memset(&dialParams,0,sizeof(dialParams));
	dialParams.dwSize = sizeof(dialParams);
	strcpy(dialParams.szEntryName  , lpszEntry         );
	stlGetStN(sCfg,2,0,0,dialParams.szPhoneNumber,sizeof(dialParams.szPhoneNumber));
	stlGetStN(sCfg,3,1,0,dialParams.szUserName   ,sizeof(dialParams.szUserName   ));
	stlGetStN(sCfg,3,2,0,dialParams.szPassword   ,sizeof(dialParams.szPassword   ));
	stlGetStN(sCfg,4,0,0,dialParams.szDomain     ,sizeof(dialParams.szDomain     ));
	stlRasHangup();		// disconnect if we had a connection before
	errNr = RasDial(NULL, NULL, &dialParams, 0, NULL, &m_ConnectionHandle); 
	if (errStr){stlFree(errStr); errStr=NULL;}
	if (errNr){
		errStr=stlInitLen(1024,0);
		RasGetErrorString(errNr,errStr->sBuf,errStr->iLen);
		stlRasHangup();
		rasMessage("Error %d stlRasDial '%s'",errNr,errStr->sBuf);
		return -5;
	}else{
		rasMessage("Connected %s",dialParams.szPhoneNumber);
	}
	return 0;
}

#endif // _WIN32
