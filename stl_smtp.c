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

#include "stl_str.h"
#include "stl_tcp.h"

#define _SmtpRxTout_	60000
#define _SmtpTxTout_	10000

/* Read response from mail server (multi-line response is supported)
**    stc     established TCP connection to mail server
** return 0 Timeout
**       >0 Server response code
*/
static int _smtpReadResponse(stlTcpConn *stc,STP *sDiag)
{
	int iRes=0;
	STP sLin,sNum;
	while (1)
	{
		sLin=stlTcpReadLine(stc,_SmtpRxTout_);
		if (sLin==NULL) return 0;
		if (sDiag) stlAppendStf(*sDiag,"<- %s\r\n",sLin->sBuf);
		if ((sLin->iLen>3)&&(sLin->sBuf[3]=='-')){stlFree(sLin);continue;}
#ifdef _DEBUG
		printf("<- %s",sLin->sBuf);
#endif
		sNum = stlGetSect(sLin,0,3);
		iRes=atoi(sNum->sBuf);
		stlFree(sNum);
		stlFree(sLin);
		break;
	}
	return iRes;
}

static int _smtpCommand(stlTcpConn *stc,STP *sDiag,char *fmt,...)
{
	STP sMsg;
	va_list ap;
	va_start(ap,fmt);
	sMsg=stlSetSta(fmt,ap);
	va_end(ap);
	stlTcpWrite(stc,(unsigned char*)(sMsg->sBuf),sMsg->iLen,_SmtpTxTout_);
	if (sDiag) stlAppendStf(*sDiag,"-> %s",sMsg->sBuf);
	stlFree(sMsg);
	return _smtpReadResponse(stc,sDiag);
}




/* Send an e-mail to the mail server
**   sHostName  Host name of mail server
**   sUname     username to login on mail server
**   sPassw     password to login on mail server
**   sFrom      e-mail adres of sender
**   sDest      list of recipients _D1 delimited
**   sEml       e-mail message to send
**   sDiag      (O) diagnostic output of SMTP dialog
** Return 0 Ok
**       -1 No connection with mail server.
**       -2 Invalid welcome response from mail server
**       -3 From adres not accepted by mail server
**       -4 One or more destination addresses rejected by server
**       -5 Server did not accept data try again later
**       -6 Communication error during data transfer
**       -7 Server did not accept data
*/
int stlSmtpSendEx(const char *sHostName,const char *sUname,const char *sPassw,const char *sFrom,STP sDest,STP sEml,STP *sDiag)
{
	int iPort,iRes,iErr=0,i,nri;
	stlTcpConn *stc;
	STP sTmp;

	if (sDiag){stlFree(*sDiag);*sDiag=stlSetSt("");}

	// Split server host name and TCP port number
	sTmp=stlSetSt(sHostName);
	iPort=stlGetDlmInt(sTmp,2,':');
	stlDelDlm(sTmp,2,':');
	if ((iPort<1)||(iPort>65535)) iPort=25;

	stc = stlTcpInit(stlTcpDnsConnectTout(sTmp->sBuf,iPort,10000));
	if (stc==NULL) stc = stlTcpInit(stlTcpDnsConnectTout(sTmp->sBuf,iPort,10000));
	stlFree(sTmp);
	if (stc==NULL) return -1;
	
	iRes=_smtpReadResponse(stc,sDiag);
	if ((iRes<200)||(iRes>=300)){iErr=-2; goto cleanup;}

	iRes=_smtpCommand(stc,sDiag,"HELO stlTool.com\r\n");
	if ((iRes<200)||(iRes>=300)){iErr=-3; goto cleanup;}

	if ((sUname)&&(sPassw)&&(sUname[0])&&(sPassw[0]))	// login 
	{
		// TODO SMTP login base 64 encode
	}

	if (_smtpCommand(stc,sDiag,"MAIL FROM: <%s>\r\n",sFrom?sFrom:"") != 250) {iErr=-3; goto cleanup;}
	
	nri = stlCount(sDest,_D1)+1;
	for (i=1;i<=nri;i++)
	{
		sTmp=stlGetStr(sDest,i,1,1);
		if (sTmp->iLen)
		{
			if (_smtpCommand(stc,sDiag,"RCPT TO: <%s>\r\n",sTmp->sBuf) != 250)
			{
				stlStoreStr("REJECT",sDest,i,2,1);
				iErr-=4;
			}else
				stlStoreStr("OK",sDest,i,2,1);
		}
		stlFree(sTmp);
	}

	// Fix mor <CR> than <LF> in e-mail message
	if (stlCount(sEml,13)>stlCount(sEml,10))
	{
		stlRemove(sEml,10);
		stlConvert(sEml,13,10);
	}
	// Fix single . on a line and make
	stlRemove(sEml,13);
	stlSwapStr(sEml,"\n.\n","\n..\n");
	// Make line-separator <CR><LF> in complete e-mail
	stlSwapStr(sEml,"\n","\r\n");
	// Start sending e-mail message
	if (_smtpCommand(stc,sDiag,"DATA\r\n",sFrom?sFrom:"") != 354){iErr=-5; goto cleanup;}
	if (stlTcpWriteNF(stc,(unsigned char*)(sEml->sBuf),sEml->iLen,_SmtpTxTout_)){iErr=-6; goto cleanup;}
	if (_smtpCommand(stc,sDiag,"\r\n.\r\n") != 250)  {iErr=-7; goto cleanup;}

	// Close SMTP session
	_smtpCommand(stc,sDiag,"QUIT\r\n");
cleanup:
	stlTcpRelease(stc);
	return iErr;
}


/* Send an e-mail to the mail server
**   sHostName  Host name of mail server 
**   sFrom      e-mail adres of sender
**   sDest      list of recipients _D1 delimited
**   sEml       e-mail message to send
** Return 0 Ok
**       -1 No connection with mail server.
**       -2 Invalid welcome response from mail server
**       -3 From adres not accepted by mail server
**       -4 One or more destination addresses rejected by server
**       -5 Server did not accept data try again later
**       -6 Communication error during data transfer
**       -7 Server did not accept data
*/
int stlSmtpSend(const char *sHostName,const char *sFrom,STP sDest,STP sEml)
{
	return stlSmtpSendEx(sHostName,NULL,NULL,sFrom,sDest,sEml,NULL);
}
/* Create plain text e-mail message
**   sSubject    subject of e-mail message
**   sFromName   Full name of sender
**   sFromEmail  e-mail adres of sender
**   sToName     Full name of recipient
**   sToEmail    e-mail adres of recipient
**   sMessage    message to send 
** Return compiled e-mail message
*/
STP stlEmailCreate(char *sSubject,char *sFromName,char *sFromEmail,char *sToName,char *sToEmail,char *sMessage)
{
	STP sEml=stlSetSt("");
	stlAppendStf(sEml,"Subject: %s\r\n",sSubject?sSubject:"");
	stlAppendStf(sEml,"From: \"%s\" <%s>\r\n",sFromName?sFromName:"",sFromEmail?sFromEmail:"");
	stlAppendStf(sEml,"To: \"%s\" <%s>\r\n",sToName?sToName:"",sToEmail?sToEmail:"");
	stlAppendStf(sEml,"\r\n");
	stlAppendSt(sEml,sMessage);
	return sEml;
}
