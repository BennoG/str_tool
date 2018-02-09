
#include "stl_ftpClient.h"

ftpClient::ftpClient(const char *host /* = NULL */,const char *user /* = NULL */,const char *pass /* = NULL */,int port /* = 0 */)
{
	initVar();
	setHost(host);
	setUser(user);
	setPass(pass);
	setPort(port);
}

ftpClient::~ftpClient()
{
	stlFree(sAnswer);
	stlFree(sHost);
	stlFree(sUser);
	stlFree(sPass);
	stlTcpRelease(stc);
}

void ftpClient::setPort(int iPortNr)
{
	if ((iPortNr > 0) && (iPortNr < 0x10000)) ftpPort = iPortNr;
}

void ftpClient::setHost(const char *host)
{
	if ((host) && (host[0]))
	{
		stlFree(sHost);
		sHost = stlSetSt(host);
		if (stlCount(sHost,':')>0)
		{
			setPort(stlGetDlmInt(sHost,2,':'));
			stlDelDlm(sHost,2,':');
		}
	}
}
void ftpClient::setPass(const char *pass)
{
	if ((pass) && (pass[0]))
	{
		stlFree(sPass);
		sPass = stlSetSt(pass);
	}
}
void ftpClient::setUser(const char *user)
{
	if ((user) && (user[0]))
	{
		stlFree(sUser);
		sUser = stlSetSt(user);
	}
}
int  ftpClient::connect(const char *host /* = NULL */,const char *user /* = NULL */,const char *pass /* = NULL */,int port /* = 0 */)
{
	setHost(host);
	setUser(user);
	setPass(pass);
	setPort(port);
	if (sHost == NULL) return -1;
	if (sUser == NULL) return -2;
	if (sPass == NULL) return -3;
	loggedIn = false;
	if (stc) stlTcpRelease(stc);
	stc = stlTcpInit(stlTcpDnsConnectTout(sHost->sBuf,ftpPort,timeoutMs));
	if (stc == NULL) return -4;
	int iRes = getAnswer();			// welcome message
	if ((iRes >= 200) && (iRes < 300)) return 0;
	printf("ftp welcome message error '%s'",sAnswer ? sAnswer->sBuf : "null");
	return -iRes;
}
int ftpClient::login(const char *user /* = NULL */,const char *pass /* = NULL */)
{
	if (loggedIn) return 0;
	setUser(user);
	setPass(pass);
	if (sUser == NULL) return -2;
	if (sPass == NULL) return -3;
	int iRes = command("USER %s",sUser->sBuf);
	if ((iRes < 200) || (iRes > 400)) return -iRes;
	if (iRes > 300) iRes = command("PASS %s",sPass->sBuf);
	if ((iRes < 200) || (iRes > 300)) return -iRes;
	loggedIn = true;
	return 0;
}

stlTcpConn *ftpClient::enterPassive()
{
	int iRes = command("TYPE I");
	iRes = command("PASV");
	if ((iRes < 200) || (iRes > 300)){printf("Error entering passive mode '%s'",sAnswer ? sAnswer->sBuf : "null"); return NULL;}
	STP sCon = stlGetDlm(sAnswer,2,'(');
	stlDelDlm(sCon,2,')');
	STP h = stlSetStf("%d.%d.%d.%d",stlGetDlmInt(sCon,1,','),stlGetDlmInt(sCon,2,','),stlGetDlmInt(sCon,3,','),stlGetDlmInt(sCon,4,','));
	int p = stlGetDlmInt(sCon,5,',') * 256 + stlGetDlmInt(sCon,6,',');
	stlTcpConn *c = stlTcpInit(stlTcpConnectTout(h->sBuf,p,timeoutMs));
	if (c == NULL) printf("faild passive ftp connection to %s:%d",h->sBuf,p);
	stlFree(h);
	stlFree(sCon);
	return c;
}

int ftpClient::setPWD(const char *dir)
{
	int iRes = command("CWD %s",dir);
	if ((iRes > 200) && (iRes < 300)) return 0;
	printf("Error setting current directory '%s'",sAnswer ? sAnswer->sBuf : "null");
	return -iRes;
}
STP ftpClient::getPWD()
{
	if (!loggedIn) return NULL;
	int iRes = command("PWD");
	if (iRes == 257)
	{
		STP sAns = stlGetDlm(sAnswer,-2,'\n');
		STP sDir = stlGetDlm(sAns,2,'"');
		if (sDir->iLen == 0)
		{
			stlFree(sDir);
			sDir = stlGetDlm(sAns,2,' ');
		}
		stlFree(sAns);
		return sDir;
	}
	return NULL;
}

static STP getFtpData(stlTcpConn *c,int iTout)
{
	if (c == NULL) return NULL;
	STP sAns = stlSetSt("");
	STP sSub = stlInitLen(10240,0);
	while (true)
	{
		int iLen = 0, iLenS = sSub->iLen;
		int iRes = stlTcpReadMax(c,sSub->sBuf,sSub->iLen,iTout,250,NULL,NULL,&iLen);
		if ((iLen > 0) && (iLen <= iLenS))
		{
			sSub->iLen = iLen;
			stlAppendStp(sAns,sSub);
			sSub->iLen = iLenS;
		}
		if (iRes < 0) break;
	}
	stlFree(sSub);
	return sAns;
}



STP ftpClient::getList(int iFormat /* = 0 */)
{
	stlTcpConn *c = enterPassive();
	if (c == NULL) return NULL;
	int iRes = command("LIST");
	if ((iRes < 100) || (iRes > 200)){
		stlTcpRelease(c); 
		printf("Error getting directory list '%s'",sAnswer ? sAnswer->sBuf : "null");
		return NULL;
	}

	STP sRes = getFtpData(c,timeoutMs);
	stlTcpRelease(c);

	iRes = getAnswer();
	if ((iRes < 200) || (iRes > 300)){
		stlFree(sRes);
		printf("Error getting directory list '%s'",sAnswer ? sAnswer->sBuf : "null");
		return NULL;
	}
	return sRes;
}

int ftpClient::command(const char *fmt,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	stlRemoveMult(pVA,"\r\n");
	stlAppendSt(pVA,"\r\n");
	stlTcpWrite(stc,(unsigned char*)(pVA->sBuf),pVA->iLen,timeoutMs);
	stlFree(pVA);
	return getAnswer();
}

/* get return from ftp server
** return < 0 error
**        > ftp code
**        -5 timeout (connection closed ?)
*/
int ftpClient::getAnswer()
{
	if (sAnswer){ stlFree(sAnswer); sAnswer = NULL; }
	while (true)
	{
		STP sLin = stlTcpReadLine(stc,timeoutMs);
		if (sLin == NULL) return -5;
		if (sAnswer){ stlAppendCh(sAnswer,'\n'); stlAppendStp(sAnswer,sLin); }
		else        { sAnswer = stlCopy(sLin); }
		if ((sLin->iLen > 4) && (sLin->sBuf[3]=='-')){ stlFree(sLin); continue; }
		int iRes = stlGetDlmInt(sLin,1,' ');
		stlFree(sLin);
		return iRes;
	}
}

STP ftpClient::download(const char *fmt,...)
{
	stlTcpConn *c = enterPassive();
	if (c == NULL) return NULL;

	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	stlRemoveMult(pVA,"\r\n");

	int iRes = command("RETR %s",pVA->sBuf);
	if ((iRes < 100) || (iRes > 200)){
		stlTcpRelease(c); 
		printf("Error getting file '%s' '%s'",pVA->sBuf,sAnswer ? sAnswer->sBuf : "null");
		stlFree(pVA);
		return NULL;
	}

	STP sRes = getFtpData(c,timeoutMs);
	stlTcpRelease(c);

	iRes = getAnswer();
	if ((iRes < 200) || (iRes > 300)){
		printf("Error getting file '%s' '%s'",pVA->sBuf,sAnswer ? sAnswer->sBuf : "null");
		stlFree(sRes);
		stlFree(pVA);
		return NULL;
	}
	return sRes;
}

/*
void testFtpClient()
{
	ftpClient CL("192.168.0.95","bercomex","bmx123");
	int iRes = CL.connect();
	iRes = CL.login();
	STP sRes = CL.getPWD();
	sRes = CL.getList();
	sRes = CL.download("/wit e de2/2013-10-10/regeling.xml");
	stlFileWrite(sRes,"dl.bin");
}
*/
