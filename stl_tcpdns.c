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
#include <ctype.h>

#ifdef _WIN32
#  include "winsock2.h"
#endif

#ifdef __linux__
#  include <netdb.h>
#  include <arpa/inet.h>
   extern int h_errno;
   typedef struct hostent * PHOSTENT;
#endif

#include "stl_str.h"
#include "stl_tcp.h"
#include "internal.h"

/* Test if IP number is valid IP number
**  sIpNumber  ip number to test
** Return 1=Valid 
**        0=Invalid
*/
int stlTcpTestValidIp(const char *sIpNumber)
{
	int i,iVa;
	STP sIP,sVA;
	sIP=stlSetSt(sIpNumber);
	if (stlConvert(sIP,'.',_D1)!=3){stlFree(sIP);return 0;}
	for (i=1;i<4;i++){
		sVA=stlGetFld(sIP,i,0,0);
		if ((sVA->iLen>3)||(sVA->iLen<0)){stlFree(sVA);stlFree(sIP);return 0;}
		iVa=atoi(sVA->sBuf);
		if ((iVa<0)||(iVa>255))          {stlFree(sVA);stlFree(sIP);return 0;}
		if (iVa<10){		// 0 - 9
			if (sVA->iLen!=1)            {stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[0])==0){stlFree(sVA);stlFree(sIP);return 0;}
		}else if (iVa<100){	// 10-99
			if (sVA->iLen!=2)            {stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[0])==0){stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[1])==0){stlFree(sVA);stlFree(sIP);return 0;}
		}else{				// 100-255
			if (sVA->iLen!=3)            {stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[0])==0){stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[1])==0){stlFree(sVA);stlFree(sIP);return 0;}
			if (isdigit(sVA->sBuf[2])==0){stlFree(sVA);stlFree(sIP);return 0;}
		}
		stlFree(sVA);
	}
	stlFree(sIP);
	return 1;
}

/* Convert DNS name to IP adres
**   sDnsName   name to resolve
**   iType      type to resolve (0=normal DNS at the moment)
** Return IP adres of DNS
**        NULL (not found or DNS error)
*/
STP stlTcpDnsResolve(const char *sDnsName,int iType)
{
	PHOSTENT phostent;
	stlTcpLibInit();
	if (stlTcpTestValidIp(sDnsName)) return stlSetSt(sDnsName);
	if ((phostent = gethostbyname (sDnsName)) == NULL) return NULL;
	if (phostent->h_length <= 0)
	{
		printf("stlTcpDnsResolve %s len=%d type=%d\n",sDnsName,phostent->h_length,phostent->h_addrtype);
		return NULL;
	}
	return stlSetSt(inet_ntoa(*(struct in_addr *)phostent->h_addr_list[0]));
}





/* Connect over TCP to IP/port adres Do DNS lookup before connect
**  sDnsName   DNS name of host to connect to
**  uPort      Port number used for connection
**  iTimeoutMs Number of ms to wait for successful connection
** return >0 socket for connection
**        -1 error create socket
**        -2 connect error
**        -3 timeout
**        -4 error getting socket status
**        -5 unknown host name
*/
int stlTcpDnsConnectTout(const char *sDnsName,unsigned short uPort,int iTimeoutMs)
{
	STP sIP;
	int iSockFd;

	sIP=stlTcpDnsResolve(sDnsName,0);
	if (sIP==NULL) return -5;
	iSockFd=stlTcpConnectTout(sIP->sBuf,uPort,iTimeoutMs);
	stlFree(sIP);
	return iSockFd;
}

/* Connect over TCP to IP/port adres Do DNS lookup before connect
**  sDnsName   DNS name of host to connect to
**  uPort      Port number used for connection
** return >0 socket for connection
**        -1 error create socket
**        -2 connect error
**        -3 timeout
**        -4 error getting socket status
**        -5 unknown host name
*/
int stlTcpDnsConnect(const char *sDnsName,unsigned short uPort)
{
	return stlTcpDnsConnectTout(sDnsName,uPort,5000);
}

