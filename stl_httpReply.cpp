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

#include "stl_httpReply.h"
#include "stdlib.h"

stlHttpReply::~stlHttpReply()
{
	int len = (int)hdr.size();
	for (int i = 0; i < len; i++)
	{
		stlFree(hdr[i]);
		hdr[i] = NULL;
	}
	len = (int) bodyParam.size();
	for (int i = 0; i < len; i++)
	{
		stlFree(bodyParam[i].sName);
		stlFree(bodyParam[i].sValue);
	}
	stlFree(body);
	body = NULL;
}

stlHttpReply::stlHttpReply(stlTcpConn *stc,bool DebugTcp /* = NULL */)
{
	initVars();
	debugTcp = DebugTcp;
	bodyCode = 0;
	body = NULL;
	httpCon = stc;
	int res1 = readHdr();
	if (res1 > 0)
	{
		int iLen = 0;
		bodyCode = stlGetDlmInt(hdr[0],2,' ');
		// "HTTP/1.1 100 Continue\r\n" message response
		if ((bodyCode == 100) && (res1 == 1)) return;	// we zijn klaar data moet nu verzonden worden.
		STP sCod = hdrGetVal("Transfer-Encoding:");
		if ((sCod) && (stricmp(sCod->sBuf,"chunked")==0))
		{
			// in chunked mode kunnen er meer delen achteraan komen.
			STP sLin = stlTcpReadLine(stc,tcpTimeout);		// lengte staat op regel in data part
			if (debugTcp) printf("stlHttpReply::stlHttpReply(1) <- '%s'",sLin ? sLin->sBuf : "NULL");
			if (sLin) iLen = strtol(sLin->sBuf,NULL,16);	// lengte is in hex zonder 0x er voor
			stlFree(sLin);
		}else
		{
			STP sLen = hdrGetVal("Content-Length:");
			if (sLen) iLen = strtol(sLen->sBuf,NULL,0);
			stlFree(sLen);
		}
		if (iLen == 0)			// geen lengte gegeven lees tot einde van de data stream.
		{
			body = stlSetSt("");
			STP sTmp = stlInitLen(9999,0);
			int iRes = 0;
			while (iRes >= 0)
			{
				sTmp->iLen = sTmp->iLenLoc - 5;
				iRes = stlTcpReadMax(stc,sTmp->sBuf,sTmp->iLen,tcpTimeout,tcpTimeout,NULL,NULL,&sTmp->iLen);
				if (sTmp->iLen > 0) stlAppendStp(body,sTmp);
			}
			stlFree(sTmp);
		}else
		{
			body = stlInitLen(iLen,0);
			int iRes = stlTcpReadMax(stc,body->sBuf,body->iLen,tcpTimeout,tcpTimeout,NULL,NULL,&iLen);
			if ((iRes < 0) || (iLen != body->iLen))
			{
				printf("stlHttpReply::stlHttpReply read res(%d) expect(%d) got(%d) bytes",iRes,body->iLen,iLen);
				tcpError = 1;
			}
		}
		if (debugTcp) printf("stlHttpReply::stlHttpReply  %d bytes",body ? body->iLen : -1);
		stlFree(sCod);
	}
}

STP stlHttpReply::hdrGet(const char *hzk)
{
	int zle = (int)strlen(hzk);
	int len = (int)hdr.size();
	for (int i = 0; i < len; i++)
	{
		if (strnicmp(hzk,hdr[i]->sBuf,zle)==0)
			return stlCopy(hdr[i]);
	}
	return NULL;
}
STP stlHttpReply::hdrGetVal(const char *hzk)
{
	STP sRow = hdrGet(hzk);
	if (sRow == NULL) return NULL;
	stlDelDlm(sRow,1,':');
	stlConvert(sRow,'\t',' ');
	stlStrip(sRow);
	return sRow;
}

int stlHttpReply::readHdr()
{
	if (httpCon==NULL) return -1;
	while (1)
	{
		STP sLin = stlTcpReadLine(httpCon,tcpTimeout);
		if (debugTcp) printf("stlHttpReply::readHdr %d <= '%s'",httpCon ? httpCon->sockFd : -99,sLin ? sLin->sBuf : "NULL");
		if ((httpCon) && (httpCon->sockFd < 0)) return -4;		// connection lost
		if (sLin==NULL) return -2;
		if (sLin->iLen == 0){stlFree(sLin); return (int) hdr.size();}
		hdr.push_back(sLin);
	}
	return -3;
}



void stlHttpReply::bodySplit()
{
	if (body == NULL) return;
	STP line = stlGetDlm(body,1,'\r');			// take only part til first \r\n
	stlRemove(line,'\n');						// remove \n if before \r for some reason
	if (stlCount(line,'?')>0)					// als antwoord res?bla=12&bla2=13 dan tot ? strippen
		stlDelDlm(line,1,'?');

	int iCnt = stlCount(line, '&') + 1;
	for (int i = 1; i <= iCnt; i++)
	{
		STP sVal = stlGetDlm(line, i, '&');
		varItem BP;
		STP sName  = stlGetDlm(sVal, 1 ,'=');
		STP sValue = stlGetDlm(sVal, 2 ,'='); 
		BP.sName  = stlUrlDecode(sName->sBuf);
		BP.sValue = stlUrlDecode(sValue->sBuf);
		stlFree(sVal);
		stlFree(sName);
		stlFree(sValue);
		bodyParam.push_back(BP);
	}
	stlFree(line);
}

const char *stlHttpReply::getBodyParamiSt(const char *bzk)
{
	if (body == NULL) return NULL;
	if (bodyParam.size() == 0) bodySplit();
	int len = (int) bodyParam.size();
	for (int i = 0; i < len; i++)
	{
		if (bodyParam[i].sName == NULL) continue;
		if (stricmp(bzk,bodyParam[i].sName->sBuf)==0)
		{
			if (bodyParam[i].sValue)
				return bodyParam[i].sValue->sBuf;
			return NULL;
		}
	}
	return NULL;
}

STP stlHttpReply::getBodyParami(const char *bzk)
{
	if (body == NULL) return NULL;
	if (bodyParam.size() == 0) bodySplit();
	int len = (int) bodyParam.size();
	for (int i = 0; i < len; i++)
	{
		if (bodyParam[i].sName == NULL) continue;
		if (stricmp(bzk,bodyParam[i].sName->sBuf)==0)
			return stlCopy(bodyParam[i].sValue);
	}
	return NULL;
}
int stlHttpReply::getBodyParamCount()
{
	if (bodyParam.size() == 0) bodySplit();
	return (int)(bodyParam.size());
}
STP stlHttpReply::getBodyParamIdxVal(int iIdx)
{
	if (bodyParam.size() == 0) bodySplit();
	int len = (int) bodyParam.size();
	if ((iIdx >= 0) && (iIdx < len)) return stlCopy(bodyParam[iIdx].sValue);
	return NULL;
}
STP stlHttpReply::getBodyParamIdxName(int iIdx)
{
	if (bodyParam.size() == 0) bodySplit();
	int len = (int) bodyParam.size();
	if ((iIdx >= 0) && (iIdx < len)) return stlCopy(bodyParam[iIdx].sName);
	return NULL;
}


