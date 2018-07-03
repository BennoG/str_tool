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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#  define _WINSOCKAPI_
#  include <windows.h>
#  include <winsock2.h>
#  include <WS2tcpip.h>
#  include <Mstcpip.h>
#  define MSG_DONTWAIT 0
#  ifdef EINPROGRESS
#     undef EINPROGRESS
#  endif
#  ifdef EWOULDBLOCK
#     undef EWOULDBLOCK
#  endif
#  define EINPROGRESS  WSAEINPROGRESS
#  define EWOULDBLOCK  WSAEWOULDBLOCK
#  define msWait(a) Sleep(a)
// forceer gebruik van deze lib
//#  pragma comment(lib, "wsock32.lib")
#  pragma comment(lib, "Ws2_32.lib")

#endif

#ifdef __linux__
#  include <signal.h>
#  include <unistd.h>
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <unistd.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  ifndef MSG_DONTWAIT
#    define MSG_DONTWAIT    0x40    /* Nonblocking io                */
#  endif
#  ifndef TCP_KEEPIDLE
#    define TCP_KEEPIDLE            4       /* Start keeplives after this period */
#  endif
#  ifndef TCP_KEEPINTVL
#    define TCP_KEEPINTVL           5       /* Interval between keepalives */
#  endif
#  ifndef TCP_KEEPCNT
#    define TCP_KEEPCNT             6       /* Number of keepalives before death */
#  endif
//#  define msWait(a) usleep((a)*1000)
#endif

#ifdef __linux__
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#endif
#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
#endif


#include "stl_str.h"
#include "stl_tcp.h"
#include "stl_crypt.h"
#include "internal.h"

int stlTcpDebug = 0;

//**************************************************************
//**
//**
//**************************************************************

#ifdef _WIN32
static void _stlTcpInit(void)
{
	static int initialized=0;
	WSADATA wsaData;
	if (initialized) return;
	initialized=1;
	//if (WSAStartup (MAKEWORD(1,1),&wsaData) != 0) {
	if (WSAStartup (MAKEWORD(2,2),&wsaData) != 0) {
		printf("TCP WinSock error %s",WSAGetLastError());
		_strSafeExit(_StTlWinsockNotFound_);
	}
}
#elif defined(__linux__)
static void _stlTcpInit(void)
{
	static int initialized=0;
	if (initialized) return;
	initialized=1;
	signal( SIGPIPE, SIG_IGN );
}
#else
static void _stlTcpInit(void)
{
}
#endif

//**************************************************************
//**
//**
//**************************************************************

/* Internal usage no need to call this
 */
void stlTcpLibInit(void)
{
	_stlTcpInit();
}


/* Connect over TCP to IP/port adres
 *  HostAddr   IP adres to connect to
 *  TcpPort    Port number used for connection
 *  iTimeoutMs Number of ms to wait for successful connection
 *	return >0 socket for connection
 *         -1 error create socket
 *         -2 connect error
 *         -3 timeout
 *         -4 error getting socket status
 */
int stlTcpConnectTout(const char *HostAddr,unsigned short TcpPort,int iTimeoutMs)
{
	int iRes,iErr,iLen;
	int sockfd;
	struct sockaddr_in dupAddr;
	_stlTcpInit();
	
	memset(&dupAddr,0,sizeof(dupAddr));
	dupAddr.sin_family      = AF_INET;
	dupAddr.sin_addr.s_addr = inet_addr(HostAddr);
	dupAddr.sin_port        = htons(TcpPort);
	
	if ( (sockfd = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error create new socket");
		return -1;
	}

	// Do actual connect stuf
	stlTcpBlock(sockfd,0);			// Disable blocking mode
	iRes=connect(sockfd, (struct sockaddr *) &dupAddr, sizeof(dupAddr));
# ifdef _WIN32
		iErr=WSAGetLastError();
# endif
# ifdef __linux__
		iErr=errno;
# endif
	stlTcpBlock(sockfd,1);			// ReEanble blocking mode
	if (iRes==0) return sockfd;	
	if ((iRes<0)&&((iErr==EINPROGRESS)||(iErr==EWOULDBLOCK))){		// Connect stil in progress
		struct timeval tv;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET((unsigned int)sockfd, &fds);
		if (iTimeoutMs<=0) iTimeoutMs=1;
		// Calculate timeout for select
		tv.tv_sec = iTimeoutMs/1000;
		tv.tv_usec=(iTimeoutMs%1000)*1000;
		iRes = select(sockfd+1,NULL, &fds, NULL,&tv);
		if (iRes< 0){stlTcpCloseSocket(sockfd); return -2;}
		if (iRes==0){stlTcpCloseSocket(sockfd); return -3;}
		iRes=-1;
		iLen=sizeof(iRes);
		iErr=getsockopt(sockfd,SOL_SOCKET,SO_ERROR,(char*)&iRes,(unsigned int*)&iLen);
		if (iErr< 0){stlTcpCloseSocket(sockfd); return -4;}
		if (iRes   ){stlTcpCloseSocket(sockfd); return -3;}	// socket is in error state
	}
	if (iRes < 0) {
		stlTcpCloseSocket(sockfd);
		return -2;
	}
	stlTcpKeepAlive(sockfd);
	return sockfd;
}

/* Connect over TCP to IP/port adres
 *  HostAddr   IP adres to connect to
 *  TcpPort    Port number used for connection
 *  iTimeoutMs Number of ms to wait for successful connection
 *	return >0 socket for connection
 *         -1 error create socket
 *         -2 connect error
 *         -3 timeout
 *         -4 error getting socket status
 */
int stlTcpConnect(const char *HostAddr,unsigned short TcpPort)
{
	return stlTcpConnectTout(HostAddr,TcpPort,5000);
}

/* Close socket
 */
void stlTcpCloseSocket(int iSockFd)
{
	if (iSockFd>=0){
#		ifdef __linux__
			close(iSockFd);
#		endif
#		ifdef _WIN32
			closesocket(iSockFd);
#		endif
	}
}


/* Turn socket blocking mode on(1) or off(0)
 *	return 0 Ok
 */ 
int stlTcpBlock(int sockFd,int onFlg)
{
# ifdef __linux__
	  int flg=fcntl(sockFd,F_GETFL);
		if (onFlg){
			flg &= ~O_NONBLOCK;
		}else{
			flg |=  O_NONBLOCK;
		}
		fcntl(sockFd,F_SETFL,flg);
#	endif
#	ifdef _WIN32
		unsigned long arg;
		if (onFlg) arg=0; else arg=1;
		ioctlsocket(sockFd,FIONBIO,&arg);
#	endif
	return 0;
}
/* set socket to send keep-alive packets at regular intervals
*   sockFd  socket to perform keep-alive on
*  return 0 Ok
*         1 Error
*/
int stlTcpKeepAlive(int sockFd)
{
	int flag = 1;
	int result = setsockopt(sockFd,	// socket affected
		SOL_SOCKET,					// set option at socket level
		SO_KEEPALIVE,				// name of option
		(char *) &flag,				// the cast is historical cruft
		sizeof(int));				// length of option value
	if (result < 0)	return 1;		// Error
	return 0;
}


int stlTcpKeepAliveParameters(int sockFd,int aantal,int interval)
{
	int iErr = 0, result = 0;
	int flag = 1,flgLen = sizeof(int);
	flag = (aantal > 0) && (interval > 0) ? 1 : 0;
	result = setsockopt(sockFd,	// socket affected
		SOL_SOCKET,					// set option at socket level
		SO_KEEPALIVE,				// name of option
		(char *) &flag,				// the cast is historical cruft
		sizeof(int));				// length of option value
	if (result < 0){ iErr = result; printf("stlTcpKeepAlive SO_KEEPALIVE error %d",result); }

#ifdef __linux__
	if (flag)	// on Linux deze zijn default 9 60 75
	{
		result = setsockopt(sockFd,SOL_TCP,TCP_KEEPCNT,&aantal,sizeof(aantal));
		//result = getsockopt(sockFd,SOL_TCP,TCP_KEEPCNT,&flag,(socklen_t*)&flgLen);
		if (result < 0){iErr = result; printf("stlTcpKeepAlive SOL_TCP,TCP_KEEPCNT(%d)(%d)(%d)",result,flag,flgLen);}

		result = setsockopt(sockFd,SOL_TCP,TCP_KEEPIDLE,&interval,sizeof(interval));
		//result = getsockopt(sockFd,SOL_TCP,TCP_KEEPIDLE,&flag,(socklen_t*)&flgLen);
		if (result < 0){iErr = result; printf("stlTcpKeepAlive SOL_TCP,TCP_KEEPIDLE(%d)(%d)(%d)",result,flag,flgLen);}

		result = setsockopt(sockFd,SOL_TCP,TCP_KEEPINTVL,&interval,sizeof(interval));
		//result = getsockopt(sockFd,SOL_TCP,TCP_KEEPINTVL,&flag,(socklen_t*)&flgLen);
		if (result < 0){iErr = result; printf("stlTcpKeepAlive SOL_TCP,TCP_KEEPINTVL(%d)(%d)(%d)",result,flag,flgLen);}
	}
#endif
#ifdef _WIN32
	if (flag)
	{
		DWORD  dwBytesRet = 0;
		struct tcp_keepalive   alive;
		alive.onoff = flag;
		alive.keepalivetime = interval;
		alive.keepaliveinterval = interval;
		if (WSAIoctl(sockFd, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),	NULL, 0, &dwBytesRet, NULL, NULL) == SOCKET_ERROR)
		{
			printf("WSAIotcl(SIO_KEEPALIVE_VALS) failed with error code %d\n", WSAGetLastError());
			return 1;
		}
	}
#endif

	if (iErr < 0)	return 1;		// Error
	return 0;
}




/* set / clear socket state to send immediately after write commando. (don't buffer(nagle) data)
*   sockFd  socket to set no-delay option on
*   onFlg   1 enable no-delay 0 disable
*  return 0 Ok
*         1 Error
*/
int stlTcpNoDelay(int sockFd,int onFlg)
{
	int flag = (onFlg)?1:0;
	int result = setsockopt(sockFd,	// socket affected
		IPPROTO_TCP,				// set option at TCP level
		TCP_NODELAY,				// name of option
		(char *) &flag,				// the cast is historical cruft
		sizeof(int));				// length of option value
	if (result < 0)	return 1;		// Error
	return 0;
}

/* Initialiseer socket structure
 *	sockFd (socket file handle to use)
 * return: socket structure
 */
struct stlTcpConn *stlTcpInit(int sockFd)
{
	struct stlTcpConn *stc;
	if (sockFd<0) return NULL;
	stc=malloc(sizeof(struct stlTcpConn));
	if (stc==NULL) return NULL;
	memset(stc,0,sizeof(struct stlTcpConn));
	stc->sockFd=sockFd;
	stc->magic =_MagicTcp_;
	stlMutexInit(&stc->muxTX);
//	stlTcpBlock(sockFd,0);
	return stc;
}

/* Clear local send en receive buffer of TCP connection
 *       This dos not clear the kernel buffers of the stream.
 * stc pointer to communication struct	
 */
void stlTcpClear(struct stlTcpConn *stc)
{
	if (stc==NULL) return;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	if (stc->sLine){stlFree(stc->sLine); stc->sLine=NULL;}
	stc->rLen=stc->rOfs=stc->xOfs=0;
}

/* Release and close socket structure
 *	stc pointer allocated from stlTcpInit
 */
int stlTcpRelease(struct stlTcpConn *stc)
{
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stc->magic = 0;
	if (stc->sLine) stlFree(stc->sLine);
	if (stc->pRxCrypt) stlCryptRelease(stc->pRxCrypt);
	if (stc->pTxCrypt) stlCryptRelease(stc->pTxCrypt);
	stlTcpCloseSocket(stc->sockFd);
	stlMutexRelease(&stc->muxTX);
	memset(stc,0,sizeof(struct stlTcpConn));
	free(stc);
	return 0;
}
/* Release but don't close socket structure
 *	stc pointer allocated from stlTcpInit
 */
int stlTcpReleaseNoClose(struct stlTcpConn *stc)
{
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stc->sockFd=-1;
	return stlTcpRelease(stc);
}


/* Start data encryption on TCP data stream.
**  stc       TCP manager structure pointer
**  pRxCrypt  (optional) encryption data for received data
**  pTxCrypt  (optional) encryption data for transmitted data
*/
void stlTcpCryptStart(stlTcpConn *stc,PSTLCRYPT pRxCrypt,PSTLCRYPT pTxCrypt)
{
	if (stc==NULL) return;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stc->pRxCrypt = pRxCrypt;
	stc->pTxCrypt = pTxCrypt;
	if ((stc->rOfs<stc->rLen)&&(stc->pRxCrypt))	// data in receive buffer description if needed
		stlCryptData(stc->pRxCrypt,stc->rBuf+stc->rOfs,stc->rLen-stc->rOfs);	// Decrypt data if data encryption is used
}

/* Get character from TCP data
 *	stc     communication structure pointer
 *  timeout timeout in ms  (when <0 wil wait forever)
 * Return >=0 character read from stream
 *        -1  NULL TCP struct error
 *        -2  Invalid TCP struct error
 *        -3  Timeout
 *        -4  Unknown error
 *        -99 Connection lost
 */
int stlTcpGetChar(struct stlTcpConn *stc,int timeout)
{
	int maxfd,r,len;
	struct timeval tv;
	fd_set fds;

	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	if (stc->sockFd<0) return -99;					// Disconnected
	if (stc->rOfs>=stc->rLen){						// Read new data from port.
		if ((stc->iMaxBps>0)&&(stc->rOfs>0)){		// Obey speed limit if it is set
			len =(stc->rOfs*1000)/stc->iMaxBps;		// Calculate number of ms to wait before receiving more data
			stlMsWait(len);
		}

		FD_ZERO(&fds);
		FD_SET((unsigned int)stc->sockFd, &fds);
		maxfd=stc->sockFd;
		if (timeout==0) timeout=1;
		// Bereken timeout voor select
		if (timeout>0){
			tv.tv_sec = timeout/1000;
			tv.tv_usec=(timeout%1000)*1000;
			r = select(maxfd+1, &fds, NULL, NULL,&tv);
		}else{
#ifdef _WIN32
			r = select(maxfd+1, &fds, NULL, NULL,NULL);		// block indefinitely.
#else
			do{		// als de socket in een andere thread werd gesloten dan kwam je nooit meer uit select (Linux SuSe 9 en 10.3)
				tv.tv_sec = 10;
				tv.tv_usec=  0;
				FD_ZERO(&fds);
				FD_SET((unsigned int)stc->sockFd, &fds);
				r = select(maxfd+1, &fds, NULL, NULL,&tv);	// block indefinitely.
				//printf("unlimited timeout loop test %d\n",r);
				if (stc->sockFd<0) return -99;			// socket is gesloten
			}while (r==0);						// timeout doen we niet aan.
#endif
		}
		if (r==0) return -3;						// Timeout
		if (r< 0) return -4;						// Unknown error
		if (stc->magic != _MagicTcp_) return -99;	// destroyed by other thread
		if (stc->sockFd < 0) return -99;			// disconnected from other thread
		if (FD_ISSET(stc->sockFd, &fds)){
			len=recv(stc->sockFd,stc->rBuf,sizeof(stc->rBuf),MSG_DONTWAIT);
			if (len==0){							// Connection lost
				stlTcpCloseSocket(stc->sockFd);
				stc->sockFd=-1;
				return -99;							// Connection lost
			}
			if (len<0) return -5;					// Unknown error
			stc->rLen=len;							// Set buffer length to number of bytes received
			stc->rOfs=0;
			if (stc->pRxCrypt) stlCryptData(stc->pRxCrypt,stc->rBuf,stc->rLen);	// Decrypt data if data encryption is used
		}else{
			return -6;								// Unknown may never happen
		}
	}		
	return (stc->rBuf[stc->rOfs++]);
}


/* Read max number of bytes
 *	stc      communication structure
 *  buf      buffer to write received chars in
 *  iLen     max number of bytes to receive
 *  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
 *  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
 *  sIgnore  string of characters to ignore in data stream.           (optional)
 *  sStop    string of chars to stop on.                              (optional)
 *  iReadCnt pointer to integer where number of read bytes is stored. (optional)
 * return: >0 sStop character hit
 *          0 Max number of bytes read
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 timeout
 *         -5 NULL receive buffer
 */
int stlTcpReadMax(struct stlTcpConn *stc,void *buf,int iLen,int timeout1,int timeout2,const char *sIgnore,const char *sStop,int *iReadCnt)
{
	char *sBuf=buf;
	int iOfs=0,iRes=0;
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	if (buf==NULL) return -5;
	while (iOfs<iLen){
		if (stc->rOfs>=stc->rLen){						// we don't have any data left
			if ((iOfs)&&(timeout2==0))					// if we processed data and timeout2 = 0 direct return.
			{
				if (iReadCnt) *iReadCnt=iOfs;							// Number read bytes
				return iRes;
			}
			if (iOfs)iRes=stlTcpGetChar(stc,timeout2);	// Timeout if we got any data
			else     iRes=stlTcpGetChar(stc,timeout1);	// Timeout for first char to arive
			if (iRes<0){
				if (iReadCnt) *iReadCnt=iOfs;							// Number read bytes
				return iRes;
			}
		}else{
			iRes=stc->rBuf[stc->rOfs++];
		}
		if ((sIgnore) && (strchr(sIgnore,iRes))) continue;	// ignore char
		if ((sStop  ) && (strchr(sStop  ,iRes))){
			if (iReadCnt) *iReadCnt=iOfs;
			return iRes;																// Stop char hit
		}
		sBuf[iOfs++]=iRes;
	}
	if (iReadCnt) *iReadCnt=iOfs;
	return 0;
}
/* Read max number of bytes
 *	stc      communication structure
 *  buf      buffer to write received chars in
 *  iLen     max number of bytes to receive
 *  timeout  time to wait for first character to arrive  (<0 wait indefinitely)
 *  iReadCnt pointer to integer where number of read bytes is stored. (optional)
 * return: >0 number of bytes read.
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 timeout
 *         -5 NULL receive buffer
 */
int stlTcpRead(struct stlTcpConn *stc,void *buf,int iLen,int timeout,int *iReadCnt)
{
	return stlTcpReadMax(stc,buf,iLen,timeout,timeout,NULL,NULL,iReadCnt);
}

/* Read string struct from TCP data stream
 *	stc      communication structure
 *  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
 *  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
 *  sIgnore  string of characters to ignore in data stream.           (optional)
 *  sStop    string of chars to stop on.                              (optional)
 * return: NULL timeout or read error
 *         Data data was read til stop char was reached
 */
STP stlTcpReadLineEx(struct stlTcpConn *stc,int timeout1,int timeout2,const char *sIgnore,const char *sStop)
{
	int iOfs,iLen,iCnt,iExtra=256,ch;
	STP Lin;
	if (stc==NULL) return NULL;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	if (stc->sockFd<0) return NULL;

	if (stc->sLine){						// We already have some data (continue using that)
		Lin=stc->sLine;						// Use that data
		stc->sLine=NULL;
		iOfs=Lin->iLen;
		iLen=iOfs+iExtra;					// Add extra space to buffer
		_strTlInsDel(Lin,iOfs,iLen);
	}else{
		iOfs=0;										// We start with new data
		iLen=iOfs+iExtra;					// Some extra space 
		Lin=_strTlInitVar(iLen);	// Initialize
	}

	while (1){
		_strTlLockVar(Lin);
		ch=stlTcpReadMax(stc,&Lin->sBuf[iOfs],iExtra,timeout1,timeout2,sIgnore,sStop,&iCnt);
		iOfs+=iCnt;
		Lin->iLen=iOfs;						// Set actual count in struct
		Lin->sBuf[iOfs]=0;				// Add terminating 0
		_strTlUnlockVar(Lin);			// Unlock
		if (ch<0) break;					// Timeout or error
		if (ch) return Lin;				// Stop character reached
		iExtra *= 2;							// Expand some more
		iLen=iOfs+iExtra;
		_strTlInsDel(Lin,iOfs,iLen);
		timeout1=timeout2;				// We have some data so timeout1 needs to be same as timeout2
	}
	stc->sLine=Lin;
	return NULL;
}

/* Read string struct from TCP data stream
 *	stc      communication structure
 *  timeout  time to wait for characters to arrive (resets after every char)(<0 wait indefinitely)
 * return: NULL timeout or read error
 *         Data line of chars was rad from TCP stream
 */
STP stlTcpReadLine(struct stlTcpConn *stc,int timeout)
{
	return stlTcpReadLineEx(stc,timeout,timeout,"\r","\n");
}




/************************************************************************/
/*                                                                      */
/* W R I T E                                                            */
/*                                                                      */
/************************************************************************/

/* Flus pending bytes in output buffer
 *	stc      communication structure
 *  timeout  time to wait before aborting
 * Return    0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpFlush(struct stlTcpConn *stc,int timeout)
{
#ifdef __linux__
	int maxfd;
	struct timeval tv;
	fd_set fds;
#endif /* __linux__ */
	unsigned char *buf;
	int res,len;

	if (stlTcpDebug & 1)
	{
		printf("stlTcpFlush a stc=%s ofs=%d sock=%d magic=0x%X timout=%d",
			stc ? "ok" : "null",
			stc ? stc->xOfs   : -98,
			stc ? stc->sockFd : -98,
			stc ? stc->magic  :   0,
			timeout);
	}


	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	if (stc->sockFd<0){stc->xOfs=0; return -99;}
	if (stc->xOfs<=0) return 0;
	buf=stc->xBuf;
	len=stc->xOfs;
	stc->xOfs=0;
	if (timeout<0 ) timeout=0x7FFFFFFF;
	if (timeout==0) timeout=1;
	
	if ((stc->iMaxBps>0)&&(len>0)){		// Er is maximaal aantal bytes per sec ingesteld
		res =(len*1000)/stc->iMaxBps;		// Aantal ms wachten.
		stlMsWait(res);
	}
	
#ifdef _WIN32
	setsockopt(stc->sockFd,			// socket affected
		SOL_SOCKET,					// set option at TCP level
		SO_SNDTIMEO,				// name of option
		(char *) &timeout,			// the cast is historical cruft
		sizeof(int));				// length of option value
#endif /* _WIN32 */

	// encrypt data if encryption is activated
	if (stc->pTxCrypt) stlCryptData(stc->pTxCrypt,buf,len);

	while(len>0){
#ifdef __linux__
		FD_ZERO(&fds);
		FD_SET((unsigned int)stc->sockFd, &fds);
		maxfd=stc->sockFd;
		// Bereken timeout voor select
		tv.tv_sec = timeout/1000;
		tv.tv_usec=(timeout%1000)*1000;
		res = select(maxfd+1,NULL, &fds, NULL,&tv);

		if (stlTcpDebug & 1) printf("stlTcpFlush select res=%d",res);

		if (res==0) return -3;
		if (res<0 ) return -4;
#endif /* __linux__ */
		res=send(stc->sockFd,buf,len,MSG_DONTWAIT);
#ifdef _WIN32
		if (res==SOCKET_ERROR)
		{
			res=WSAGetLastError();
			if (res==WSAETIMEDOUT) return -3;
			printf("Res=%d sockFd=%d\n",res,stc->sockFd);
			res=-1;
		}
#endif /* _WIN32 */

		if (stlTcpDebug & 1) printf("stlTcpFlush send res=%d len=%d",res,len);

		if (res<=0){									// some kind of error
				stlTcpCloseSocket(stc->sockFd);
				stc->sockFd=-1;
				return -99;
		}
		buf+=res;
		len-=res;
	}
	return 0;
}

/* Dit is een macro geworden daar anders de write functies zeer traag werden.
 *	Er werd dan voor een groot block data telkens een functie aangeroepen.
 *  max data transfer was 50kB/sec op 1Ghz machine
 */
#define stlTcpPutChar_I(stc,ch,timeout) {stc->xBuf[stc->xOfs++]=ch; if (stc->xOfs>=(int)sizeof(stc->xBuf)){int __res=stlTcpFlush(stc,timeout); if (__res<0){ stlMutexUnlock(&stc->muxTX); return __res;}}}

/* Write one char to TCP connection
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpPutChar(struct stlTcpConn *stc,char ch,int timeout)
{
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stc->xBuf[stc->xOfs++]=ch; 
	if (stc->xOfs>=(int)sizeof(stc->xBuf))
	{
		int __res=stlTcpFlush(stc,timeout); 
		if (__res<0) return __res;
	}
	return 0;
}

/* Write block of data to TCP connection (Flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWrite(struct stlTcpConn *stc,const unsigned char *buf,int len,int timeout)
{
	int i;
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stlMutexLock(&stc->muxTX);
	for (i=0;i<len;i++) stlTcpPutChar_I(stc,buf[i],timeout);
	i=stlTcpFlush(stc,timeout);
	stlMutexUnlock(&stc->muxTX);
	return i;
}

/* Write TCP connection like printf (Flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWritef(struct stlTcpConn *stc,int timeout,const char *fmt,...)
{
	STP sBuf;
	va_list ap;
	int iRes;
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();

	va_start(ap,fmt);
	sBuf=stlSetSta(fmt,ap);
	va_end(ap);
	if (sBuf==NULL) return -4;
	iRes=stlTcpWrite(stc,(unsigned char *)(sBuf->sBuf),sBuf->iLen,timeout);
	stlFree(sBuf);
	return iRes;
}

/* Write block of data to TCP connection (don't flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWriteNF(struct stlTcpConn *stc,unsigned char *buf,int len,int timeout)
{
	int i;
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();
	stlMutexLock(&stc->muxTX);
	for (i=0;i<len;i++) stlTcpPutChar_I(stc,buf[i],timeout);
	stlMutexUnlock(&stc->muxTX);
	return 0;
}

/* Write TCP connection like printf (don't flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWriteNFf(struct stlTcpConn *stc,int timeout,const char *fmt,...)
{
	STP sBuf;
	va_list ap;
	int iRes;
	if (stc==NULL) return -1;
	if (stc->magic != _MagicTcp_) _lxBreak_();

	va_start(ap,fmt);
	sBuf=stlSetSta(fmt,ap);
	va_end(ap);
	if (sBuf==NULL) return -4;
	iRes=stlTcpWriteNF(stc,(unsigned char *)(sBuf->sBuf),sBuf->iLen,timeout);
	stlFree(sBuf);
	return iRes;
}


/* Connect 2 sockets together with optional logging capabilities
 *  iSock1  First socket to use
 *  iSock2  Second socket to use
 *  iByteUp (optional) bytes from iSock1 -> iSock2
 *  iByteDn (optional) bytes from iSock2 -> iSock1 (may be same as iByteUp)
 *  fUp     (optional) FILE to write data from iSock1 -> iSock2
 *  fDn     (optional) FILE to write data from iSock2 -> iSock1 (may be same as fUp)
 */
void stlTcpConnectSocketsLog(int iSock1,int iSock2,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn)
{
	stlTcpConnectSocketsLogCrypt(iSock1,iSock2,iByteUp,iByteDown,fUp,fDn,NULL,NULL,NULL,NULL);
}

int connectSocketTimeoutSec = 0;

void stlTcpConnectSocketsLogCrypt(int iSock1,int iSock2,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn,PSTLCRYPT pRxCry1,PSTLCRYPT pTxCry1,PSTLCRYPT pRxCry2,PSTLCRYPT pTxCry2)
{
	struct timeval tv;
	int  high_f=0,res,len1=0,len2=0,ofs1=0,ofs2=0;
	char buf1[2048];
	char buf2[2048];

	fd_set  readfs,wrtefs,errfs;
	if ((iSock1<0)||(iSock2<0)) return;

	stlTcpBlock(iSock1,0);	// force non blocking mode
	stlTcpBlock(iSock2,0);	// force non blocking mode
	while (1){
		FD_ZERO(&readfs);
		FD_ZERO(&wrtefs);
		FD_ZERO(&errfs );
		// read write select settings
		if (len1)FD_SET((unsigned int)iSock1,&wrtefs);
		else     FD_SET((unsigned int)iSock2,&readfs);
		if (len2)FD_SET((unsigned int)iSock2,&wrtefs);
		else     FD_SET((unsigned int)iSock1,&readfs);
		// error select settings
		FD_SET((unsigned int)iSock1,&errfs); 
		FD_SET((unsigned int)iSock2,&errfs); 
		if (iSock1>high_f) high_f=iSock1;
		if (iSock2>high_f) high_f=iSock2;
		if (connectSocketTimeoutSec > 0)
		{
			tv.tv_sec = connectSocketTimeoutSec;
			tv.tv_usec= 0;
			res=select(high_f+1, &readfs, &wrtefs , &errfs , &tv);
			if (res == 0)
			{
				printf("stlTcpConnectSocketsLogCrypt timeout");
				break;
			}
		}else
			res=select(high_f+1, &readfs, &wrtefs , &errfs , NULL);
		if (res<0) break;
		if (FD_ISSET(iSock1,&errfs)) break;	// disconnect on error
		if (FD_ISSET(iSock2,&errfs)) break;	// disconnect on error

		if (len1)	// stil need to write some data to iSock1
		{
			if (FD_ISSET(iSock1,&wrtefs)){
				res=send(iSock1,buf1+ofs1,len1,0);
				if (res>0){len1-=res;ofs1+=res;if (len1<=0) len1=0;}
				if (res<0) stlMsWait(50);
				//if (res<0) break;
			}
		}
		else
		{
			if (FD_ISSET(iSock2,&readfs)){
				len1=recv(iSock2,buf1,sizeof(buf1),0);
				if (len1>0){
					if (pRxCry2) stlCryptData(pRxCry2,buf1,len1);	// Decrypt data if data encryption is used
					if (pTxCry1) stlCryptData(pTxCry1,buf1,len1);	// Encrypt data if data encryption is used
					ofs1=0;
					res =send(iSock1,buf1+ofs1,len1,0);
					if (iByteDown) *iByteDown=*iByteDown+len1;
					if (fDn){fwrite(buf1,1,len1,fDn);fflush(fDn);}
					if (res>0){len1-=res;ofs1+=res;if (len1<=0) len1=0;}
				}else
					break;
			}
		}
		
		if (len2)
		{
			if (FD_ISSET(iSock2,&wrtefs)){
				res=send(iSock2,buf2+ofs2,len2,0);
				if (res>0){len2-=res;ofs2+=res;if (len2<=0) len2=0;}
				if (res<0) stlMsWait(50);
				//if (res<0) break;
			}
		}
		else
		{
			if (FD_ISSET(iSock1,&readfs)){
				len2=recv(iSock1,buf2,sizeof(buf2),0);
				if (len2>0){
					if (pRxCry1) stlCryptData(pRxCry1,buf2,len2);	// Decrypt data if data encryption is used
					if (pTxCry2) stlCryptData(pTxCry2,buf2,len2);	// Encrypt data if data encryption is used

					ofs2=0;
					res =send(iSock2,buf2+ofs2,len2,0);
					if (iByteUp) *iByteUp=*iByteUp+len2;
					if (fUp){fwrite(buf2,1,len2,fUp);fflush(fUp);}
					if (res>0){len2-=res;ofs2+=res;if (len2<=0) len2=0;}
				}else
					break;
			}
		}
	}
}



/* Connect 2 sockets together with optional logging capabilities
 *	stc1    First connection
 *  stc2    Second connection
 *  iByteUp (optional) bytes from iSock1 -> iSock2
 *  iByteDn (optional) bytes from iSock2 -> iSock1 (may be same as iByteUp)
 *  fUp     (optional) FILE to write data from iSock1 -> iSock2
 *  fDn     (optional) FILE to write data from iSock2 -> iSock1 (may be same as fUp)
 */
void stlTcpConnectConsLog(struct stlTcpConn *stca,struct stlTcpConn *stcb,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn)
{
	if ((stca==NULL)||(stcb==NULL)) return;
	if (stca->rLen > stca->rOfs){
		unsigned char *p=&stca->rBuf[stca->rOfs];
		int   l=stca->rLen-stca->rOfs;
		if (stlTcpWrite(stcb,p,l,60000)<0) return;
		if (iByteUp) *iByteUp=*iByteUp+l;
		if (fUp){fwrite(p,1,l,fUp);fflush(fUp);}
	}
	if (stcb->rLen > stcb->rOfs){
		unsigned char *p=&stcb->rBuf[stcb->rOfs];
		int   l=stcb->rLen-stcb->rOfs;
		if (stlTcpWrite(stca,p,l,60000)<0) return;
		if (iByteDown) *iByteDown=*iByteDown+l;
		if (fDn){fwrite(p,1,l,fDn);fflush(fDn);}
	}
	stlTcpConnectSocketsLogCrypt(stca->sockFd,stcb->sockFd,iByteUp,iByteDown,fUp,fDn,stca->pRxCrypt,stca->pTxCrypt,stcb->pRxCrypt,stcb->pTxCrypt);
}

/* Connect 2 TCP connections together
 *	stc1    First connection
 *  stc2    Second connection
 */
void stlTcpConnectCons(struct stlTcpConn *stca,struct stlTcpConn *stcb)
{
	stlTcpConnectConsLog(stca,stcb,NULL,NULL,NULL,NULL);
}


/************************************************************************/
/* UDP functions                                                        */
/************************************************************************/

#define _MagicUdp_	0x21A5125A

/* Close UDP port and release its resources
*/
void stlUdpRelease(stlUdpConn *suc)
{
	if (suc==NULL) return;
	if (suc->magic != _MagicUdp_) _lxBreak_();
	suc->magic = 0;
	if (suc->sockFd>0) stlTcpCloseSocket(suc->sockFd);
	memset(suc,0,sizeof(stlUdpConn));
	free(suc);
}

/* Create a new UDP connection
**   RemoteAddr    (opt) remote IP to send the data to.
**   UdpRemotePort (opt) remote UDP to send the data to.
**   UdpLocalPort  (opt) local UDP port to listen on.
** Return NULL error
**        ptr  struct to send and receive data on.
*/
stlUdpConn * stlUdpConnectIp(const char *RemoteAddr,unsigned short UdpRemotePort,unsigned short UdpLocalPort)
{
	int iParam;
	struct sockaddr_in sAddr;
	stlUdpConn *suc;
	_stlTcpInit();
	suc=malloc(sizeof(stlUdpConn));
	memset(suc,0,sizeof(stlUdpConn));
	if ((RemoteAddr)&&(RemoteAddr[0])) strncpy(suc->sRemoteHost,RemoteAddr,sizeof(suc->sRemoteHost)-1);

	suc->uRxPort=UdpLocalPort;
	suc->uTxPort=UdpRemotePort;
	suc->magic  = _MagicUdp_;
	suc->sockFd = (int)socket(AF_INET, SOCK_DGRAM, 0);

	// Open a UDP socket (an Internet datagram socket).
	if (suc->sockFd < 0) {
		printf("Error: Can not open UDP socket %s\n",strerror(errno));
		stlUdpRelease(suc);
		return NULL;
	}

	// Set ReuseAddr option 
	iParam=1;
	if (setsockopt(suc->sockFd, SOL_SOCKET,SO_REUSEADDR,(char*)&iParam, sizeof(iParam))<0)
		printf("Warning: UDP connect can't set socket option SO_REUSEADDR %s\n",strerror(errno));

	// Set the BROADCAST option so we can do broadcasting UDP
	iParam=1;
	if (setsockopt (suc->sockFd , SOL_SOCKET, SO_BROADCAST, (char *)&iParam, sizeof iParam) < 0)
		printf("Warning: UDP connect can't set socket option SO_BROADCAST %s\n",strerror(errno));

	if (UdpLocalPort){
		memset(&sAddr,0,sizeof(sAddr));
		sAddr.sin_family      = AF_INET;
		sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sAddr.sin_port        = htons(UdpLocalPort);
		if (bind(suc->sockFd, (struct sockaddr *) &sAddr, sizeof(sAddr)) < 0) {
			printf("Error: UDP connect cant bind to port %d\n",UdpLocalPort);
			stlUdpRelease(suc);
			return NULL;
		}
	}
	return suc;
}
int stlUdpBind(stlUdpConn *suc,const char *LocalAddr,unsigned short UdpLocalPort)
{
	struct sockaddr_in sAddr;
	if ((suc == NULL) || (LocalAddr == NULL) || (LocalAddr[0] == 0) /*|| (UdpLocalPort == 0) */) return -2;	// als UdpLocalPort==0 dan is het auto UDP port aan onze kant
	if (suc->magic != _MagicUdp_) _lxBreak_();
	memset(&sAddr,0,sizeof(sAddr));
	sAddr.sin_family      = AF_INET;
	sAddr.sin_addr.s_addr = inet_addr(LocalAddr);
	sAddr.sin_port        = htons(UdpLocalPort);
	if (bind(suc->sockFd, (struct sockaddr *) &sAddr, sizeof(sAddr)) < 0) {
		printf("Error: UDP connect cant bind to port %d\n",UdpLocalPort);
		return -1;
	}
	return 0;
}

int stlUdpBindMC(stlUdpConn *suc,const char *LocalAddr,unsigned short UdpLocalPort,const char *McastAddr)
{
	int iRes;
	struct ip_mreq {
		struct in_addr imr_multiaddr;   /* IP multicast address of group */
		struct in_addr imr_interface;   /* local IP address of interface */
	}mreq;
	struct sockaddr_in sAddr;
	if ((suc == NULL) || (LocalAddr == NULL) || (LocalAddr[0] == 0) || (UdpLocalPort == 0)) return -2;
	if (suc->magic != _MagicUdp_) _lxBreak_();
	memset(&sAddr,0,sizeof(sAddr));
	sAddr.sin_family      = AF_INET;
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sAddr.sin_port        = htons(UdpLocalPort);
	if (bind(suc->sockFd, (struct sockaddr *) &sAddr, sizeof(sAddr)) < 0) {
		printf("Error: UDP connect cant bind to port %d\n",UdpLocalPort);
		return -1;
	}
	memset(&mreq,0,sizeof(mreq));
	mreq.imr_interface.s_addr = inet_addr(LocalAddr);
	mreq.imr_multiaddr.s_addr = inet_addr(McastAddr);
	iRes = setsockopt(suc->sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
#ifdef __linux__
	if (iRes < 0)
	{
		printf("UDP setsockopt error %d(%s)",errno,strerror(errno));
		return -1;
	}
#endif
#ifdef _WIN32
	if (iRes == SOCKET_ERROR)
	{
		iRes=WSAGetLastError();
		printf("UDP setsockopt error %d",iRes);
		return -1;
	}
#endif
	return 0;
}

stlUdpConn * stlUdpConnect(void)
{
	return stlUdpConnectIp(NULL,0,0);
}

/* Send UDP data to remote site
**    suc         struct from stlUdpConnect
**    sRemoteIP   (opt) remote IP to send data to
**    uRemotePort (opt) remote port number to send data to
**    sData       data to send to remote site
**  Return >0 number of bytes send
**        -1 NULL stlUdpConn data
**        -2 Corrupted stlUdpCon data
**        -3 Unconnected stlUdpCon data
**        -4 No remote IP info to send data
**        -5 No remote port number to send data
**        -6 No data to send to remote host
*/
int stlUdpSendEx(stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,STP sData)
{
	struct sockaddr_in cltAddr;
	if (suc==NULL) return -1;
	if (suc->magic != _MagicUdp_) _lxBreak_();
	//if (suc->magic != _MagicUdp_) return -2;
	if (suc->sockFd <=0) return -3;
	if ((sRemoteIP==NULL)||(sRemoteIP[0]==0)) sRemoteIP=suc->sRemoteHost;
	if (uRemotePort ==0) uRemotePort=suc->uTxPort;
	if (sRemoteIP[0]==0) return -4;
	if (uRemotePort ==0) return -5;
	if (sData==NULL) return -6;
	// all checks are done we can send data

	memset(&cltAddr,0,sizeof(cltAddr));
	cltAddr.sin_family      = AF_INET;
	cltAddr.sin_port        = htons(uRemotePort);
	cltAddr.sin_addr.s_addr = inet_addr(sRemoteIP);
	return sendto(suc->sockFd,sData->sBuf,sData->iLen,0,(struct sockaddr *)&cltAddr,sizeof(cltAddr));
}
int stlUdpSendExv(stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,const void *pData,int iDataLen)
{
	int iRes=-6;
	STP sData;
	sData=stlInitLen(iDataLen,0);
	if (sData)
	{
		memcpy(sData->sBuf,pData,iDataLen);
		iRes = stlUdpSendEx(suc,sRemoteIP,uRemotePort,sData);
		stlFree(sData);
	}
	return iRes;
}
int stlUdpSendExf(stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,char *fmt,...)
{
	STP sData;
	va_list ap;
	int iRes;

	va_start(ap,fmt);
	sData=stlSetSta(fmt,ap);
	va_end(ap);

	iRes=stlUdpSendEx(suc,sRemoteIP,uRemotePort,sData);
	stlFree(sData);
	return iRes;
}
int stlUdpSend(stlUdpConn *suc,STP sData)
{
	return stlUdpSendEx(suc,NULL,0,sData);
}
int stlUdpSendf(stlUdpConn *suc,const char *fmt,...)
{
	STP sData;
	va_list ap;
	int iRes;

	va_start(ap,fmt);
	sData=stlSetSta(fmt,ap);
	va_end(ap);

	iRes=stlUdpSendEx(suc,NULL,0,sData);
	stlFree(sData);
	return iRes;
}
/* Receive data from UDP socket
**    suc         struct from stlUdpConnect
**    iMsTimeout  time in ms to wait for data <0 wait unlimited
**    piErr       (opt) error number in case of NULL return
** Return Data    Data received from UDP port
**        NULL    in case of timeout (*piErr=4) or error (*piErr>0)
*/
STP stlUdpRecv(stlUdpConn *suc,int iMsTimeout,int *piErr)
{
	int iLen,iErr=0;
	struct sockaddr_in pInAdr;
	STP sData=NULL;

	if (suc==NULL){if (piErr) *piErr=1; return NULL;}
	if (suc->magic != _MagicUdp_) _lxBreak_();
//	if (suc->magic != _MagicUdp_){if (piErr) *piErr=2; return NULL;}
	if (suc->sockFd <=0) {if (piErr) *piErr=3; return NULL;}
	// all checks are done we can send data

	sData=stlInitLen(2000,0);
	while (iErr==0)
	{
		iLen=sizeof(pInAdr);
		memset(&pInAdr,0,iLen);
		if (iMsTimeout){
			fd_set  readfs;
			struct timeval sTimeOut;
			sTimeOut.tv_sec = iMsTimeout/1000;
			sTimeOut.tv_usec=(iMsTimeout%1000)*1000;
			FD_ZERO(&readfs);
			FD_SET(suc->sockFd,&readfs);
			if (iMsTimeout<0)
			{
				if (select(suc->sockFd+1,&readfs, NULL, NULL, NULL)==0) iErr=4;
			}else
			{
				if (select(suc->sockFd+1,&readfs, NULL, NULL, &sTimeOut)==0) iErr=4;
			}
		}
		if (iErr) break;
		iLen=recvfrom(suc->sockFd,sData->sBuf,sData->iLen,0,(struct sockaddr *)&pInAdr,(unsigned int*)(&iLen));
		if (iLen>=0){
			sData->iLen=iLen;
			strncpy(suc->sRemoteHost,inet_ntoa(pInAdr.sin_addr),sizeof(suc->sRemoteHost)-1);
			suc->uTxPort=ntohs(pInAdr.sin_port);
			break;
		}
#ifdef _WIN32
		if (iLen == SOCKET_ERROR)
		{
			iErr=WSAGetLastError();
			if (iErr==WSAECONNRESET){iErr=0; stlMsWait(1); continue;}	// deze komt voor UDP sockets vaak voor
			printf("UDP recvfrom error %d",iErr);
			iErr=5;
			break;
		}
#endif
#ifdef __linux__
		if (iLen<0)
		{
			stlFree(sData);
			sData=NULL;
			iErr=5;
			break;
		}
#endif
	}
	if (iErr){stlFree(sData); sData=NULL;}
	if (piErr) *piErr=iErr;
	return sData;
}

/* Start UDP server (only returns in case of error)
**    suc         struct from stlUdpConnect
**    UdpCallback Callback function when data on UDP is received.
**    pUserData   user defined data witch is forwarded to the callback function
** Return -1 NULL stlUdpConn data
**        -2 Corrupted stlUdpCon data
**        -3 Unconnected stlUdpCon data
**        -4 No local UDP port defined
*/
int stlUdpServer(stlUdpConn * suc,void (*UdpCallback)(stlUdpConn * suc,STP sRxData,void *pUserData),void *pUserData)
{
	int iErr=0;
	STP sRxData;
	if (suc==NULL) return -1;
	if (suc->magic != _MagicUdp_) _lxBreak_();
	//if (suc->magic != _MagicUdp_) return -2;
	if (suc->sockFd <=0) return -3;
	//if (suc->uRxPort==0) return -4;
	// all checks are done we can send data
	while (iErr==0)
	{
		sRxData=stlUdpRecv(suc,-1,&iErr);
		if (sRxData)
		{
			if (UdpCallback) UdpCallback(suc,sRxData,pUserData);
			stlFree(sRxData);
		}else{
			printf("stlUdpServer receive error %d",iErr);
			stlMsWait(1000);
		}
	}
	return iErr;
}

