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

#include "stl_httpClient.h"
#include "stl_str.h"
#include "stl_tcp.h"

#include <vector>

/************************************************************************/
/*                                                                      */
/************************************************************************/

/* Alle gebruikte resources vrij geven
*/
stlHttpClient::~stlHttpClient()
{
	int len = (int) hdr.size();
	for (int i = 0; i < len; i++)
	{
		stlFree(hdr[i]);
		hdr[i] = NULL;
	}
	if (hHost  ){stlFree(hHost);   hHost   = NULL;}
	if (hHostBu){stlFree(hHostBu); hHostBu = NULL;}
	if (hGet   ){stlFree(hGet);    hGet    = NULL;}
	if (ans) delete ans;
	if (stc) stlTcpRelease(stc);
}

stlHttpClient::stlHttpClient()
{
	initVars();
}
stlHttpClient::stlHttpClient(const char * url)
{
	initVars();
	setURL(url);
}

void stlHttpClient::setURL(const char *url)
{
	if (hGet) stlFree(hGet);
	hGet = stlSetSt(url);
	// strip http:// als dat er voor staat
	if (strnicmp(hGet->sBuf,"http://",7)==0){ stlInsDel(hGet,0,-7); poortNR = 80; }
	if (hHost) stlFree(hHost);
	hHost = stlGetDlm(hGet,1,'/');	// host naam uit url halen.
	stlInsDel(hGet,0,-hHost->iLen);	// enkel path over houden.
	if (stlCount(hHost,':'))		// kijk of we een poort nummer hebben dat dan ook bewaren
	{
		poortNR = stlGetDlmInt(hHost,2,':');
		stlDelDlm(hHost,2,':');
	}
}
void stlHttpClient::setHost(const char * sHost)
{
	if (hHost) stlFree(hHost);
	hHost = stlSetSt(sHost);
	if (stlCount(hHost,':'))		// kijk of we een poort nummer hebben dat dan ook bewaren
	{
		poortNR = stlGetDlmInt(hHost,2,':');
		stlDelDlm(hHost,2,':');
	}
}
void stlHttpClient::setHostBackup(const char * sHost)
{
	if (hHostBu) stlFree(hHostBu);
	hHostBu = stlSetSt(sHost);
	if (stlCount(hHostBu,':'))		// kijk of we een poort nummer hebben dat dan ook bewaren
	{
		poortNrBu = stlGetDlmInt(hHostBu,2,':');
		stlDelDlm(hHostBu,2,':');
	}else poortNrBu = poortNR;
}

void stlHttpClient::updateTag(const char * sTag,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,sTag);
	pVA=stlSetSta(sTag,ap);
	va_end(ap);
	stlRemoveMult(pVA,"\r\n");
	int idx = stlIndexOfN(pVA,':',1);
	if (idx < 0) idx = pVA->iLen;
	if (idx > 0)
	{
		int len = (int) hdr.size();
		for (int i = 0; i < len; i++)
		{
			if (hdr[i]->iLen < idx) continue;
			if (memcmp(hdr[i]->sBuf, pVA->sBuf, idx + 1) == 0)
			{
				stlExchange(hdr[i],pVA);
				stlFree(pVA);
				return;
			}
		}
	}
	hdr.push_back(pVA);
}
void stlHttpClient::addTag(const char * sHdr,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,sHdr);
	pVA=stlSetSta(sHdr,ap);
	va_end(ap);
	stlRemoveMult(pVA,"\r\n");
	hdr.push_back(pVA);
}

STP stlHttpClient::constructHeader(const char *fmt,...)
{
	STP sHdr;
	va_list ap;
	va_start(ap,fmt);
	sHdr=stlSetSta(fmt,ap);
	va_end(ap);
	stlRemoveMult(sHdr,"\r\n");
	stlAppendSt(sHdr,"\r\n");
	if ((isBackupHost) && (hHostBu))
	{
		if ((poortNrBu > 0) && (poortNrBu != 80))
			stlAppendStf(sHdr,"HOST: %s:%d\r\n",hHostBu->sBuf,poortNrBu);
		else
			stlAppendStf(sHdr,"HOST: %s\r\n",hHostBu->sBuf);
	} else if (hHost)
	{
		if ((poortNR > 0) && (poortNR != 80))
			stlAppendStf(sHdr,"HOST: %s:%d\r\n",hHost->sBuf,poortNR);
		else
			stlAppendStf(sHdr,"HOST: %s\r\n",hHost->sBuf);
	}
	int len = (int) hdr.size();
	for (int i = 0; i < len; i++)
		stlAppendStf(sHdr,"%s\r\n",hdr[i]->sBuf);
	if (keepAlive)
	{
		stlAppendSt(sHdr,"Expect: 100-continue\r\n");
		stlAppendSt(sHdr,"Connection: Keep-Alive\r\n");
	}
	return sHdr;
}
/* maak TCP verbinding aan de hand van ingegeven parameters
*/
int stlHttpClient::connect()
{
	if (stc != NULL) return 0;			// was al verbinding
	if ((hHost == NULL) || (hHost->iLen == 0)) return -1;

	if ((poortNR <= 0) || (poortNR > 0xFFFF)) poortNR = 80;
	stc = stlTcpInit(stlTcpDnsConnectTout(hHost->sBuf,poortNR,tcpTimeout));
	isBackupHost = false;
	if ((stc == NULL) && (hHostBu != NULL)){
		if ((poortNrBu <= 0) || (poortNrBu > 0xFFFF)) poortNrBu = 80;
		stc = stlTcpInit(stlTcpDnsConnectTout(hHostBu->sBuf,poortNrBu,tcpTimeout));
		isBackupHost = (stc != NULL);
	}

	if (stc) return 1;
	return -2;
}

/* generate HTTP POST message
** return 0 ok
**        <0 error
*/
int stlHttpClient::post(unsigned char *data,int datalen)
{
	if (connect() < 0) return -1;
	if (ans){ delete ans; ans = NULL;}

	int iErrCnt = 0;
	while (true)
	{
		STP sHdr = constructHeader("POST %s HTTP/%d.%d","/",vMayor,vMinor);
		stlAppendStf(sHdr,"Content-Length: %d\r\n",datalen);
		stlAppendSt(sHdr,"\r\n");

		int iRes = stlTcpWrite(stc,(unsigned char *)sHdr->sBuf,sHdr->iLen,tcpTimeout);
		if (debugTcp) printf("stlHttpClient::post(%d) %d -> '%s'",iRes,stc ? stc->sockFd : -99,sHdr->sBuf);
		stlFree(sHdr);

		stlHttpReply postAnswer(stc,debugTcp);
		if ((keepAlive) && (stc) && (stc->sockFd < 0) && (iErrCnt < 2))
		{
			if (debugTcp) printf("stlHttpClient::post reconnect");
			stlTcpRelease(stc); 
			stc=NULL;
			if (connect() < 0) return -3;		// connection lost
			iErrCnt++;
			continue;
		}

		if (postAnswer.bodyCode != 100)
		{
			iRes = postAnswer.bodyCode > 0 ? - postAnswer.bodyCode : -2;
			return iRes;
		}
		iRes = stlTcpWrite(stc,data,datalen,tcpTimeout);
		if (debugTcp) printf("stlHttpClient::post(%d) %d-> '%s'",iRes,stc ? stc->sockFd : -99,data);
		return iRes;
	}
}
stlHttpReply *stlHttpClient::read(bool consume /* = false */)
{
	stlHttpReply *r = new stlHttpReply(stc,debugTcp);
	if (!consume)	// bewaren zodat we zelf vrij kunnen geven.
	{
		if (ans) delete ans;
		ans = r;
	}
	return r;
}


int stlHttpClient::execute()
{
	if (connect() < 0) return -1;
	if (ans){ delete ans; ans = NULL;}
	STP req = constructHeader("GET %s HTTP/%d.%d\r\n",hGet->sBuf,vMayor,vMinor);
	stlAppendStf(req,"\r\n");
	if (debugTcp) printf("stlHttpClient::execute -> '%s'",req->sBuf);
	stlTcpWrite(stc,(unsigned char *)req->sBuf,req->iLen,tcpTimeout);
	read();
	stlTcpRelease(stc); stc = NULL;
	if (ans) return ans->bodyCode;
	return -2;
}

void stlHttpClient::urlAppend(const char *fmt,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	stlAppendStp(hGet,pVA);
	stlFree(pVA);
}

