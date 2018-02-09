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
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "internal.h"
#include "stl_thread.h"
#include "stl_str.h"
#include "stl_tcp.h"

#ifdef __linux__
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <arpa/inet.h>
#endif

#define _MaxServers_		20				// Maximum number of different services to run

//***************************************************************************************
//**
//**  Connection database
//**
//***************************************************************************************

/* Internal database to organize my data
 */
struct srvrDataInfo{
	unsigned short iPortNr;	// Port number to listen on
	int sockFd;							// Allocated socket number
	void *pConfig;					// User config data
	void(*proces)(struct stlTcpConInfo*);
	struct sockaddr_in serv_addr;
	int iFlags;							// Status flags
};

static struct srvrDataInfo lSvrData[_MaxServers_];

/************************************************************************/
/*  Internal functions                                                  */
/************************************************************************/

#define _LstSrvHaveSock_		0x10000		// Socket is allocated
#define _LstSrvBound_				0x20000		// Socket is bound to port
#define _LstSrvInitOk_			0x40000		// Socket ready to use
#define _LstSrvStopListen_	0x80000		// Stop listening to port

/* Initialize socket to be used as a server
 *	Keeps internal programs 
 * return 0 ok
 *       -1 No port number given
 *       -2 Can not allocate socket
 *       -3 Can not bind port
 */
static int _stlTcpSrvInitSocket(int iIdx)
{
	if (lSvrData[iIdx].iPortNr==0) return -1;							// Unknown port number
	if (lSvrData[iIdx].iFlags & _LstSrvInitOk_) return 0;	// Ok al gedaan

	if (lSvrData[iIdx].iFlags & _stlTcpSrvConectPrint_)
		printf("start initializing port %d\n",lSvrData[iIdx].iPortNr);

	if ((lSvrData[iIdx].iFlags & _LstSrvHaveSock_)==0){
		// Open a TCP socket (an Internet stream socket).
		if ((lSvrData[iIdx].sockFd = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("server: can't open stream socket %s\n",strerror(errno));
			return -2;
		}
		lSvrData[iIdx].iFlags |= _LstSrvHaveSock_;

		// Set reuse status when needed
		if (lSvrData[iIdx].iFlags & _stlTcpSrvReuseSock_){
			int res,reuse=1;
			res=setsockopt(lSvrData[iIdx].sockFd, SOL_SOCKET,SO_REUSEADDR,(char*)&reuse, sizeof(reuse));
			if (res) printf("server: can't set socket option SO_REUSEADDR %s\n",strerror(errno));
		}
	}
	if ((lSvrData[iIdx].iFlags & _LstSrvBound_)==0){
		// Bind our local address so that the client can send to us.
		lSvrData[iIdx].serv_addr.sin_family      = AF_INET;
		lSvrData[iIdx].serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		lSvrData[iIdx].serv_addr.sin_port        = htons(lSvrData[iIdx].iPortNr);
		if (bind(lSvrData[iIdx].sockFd, (struct sockaddr *) &lSvrData[iIdx].serv_addr, sizeof(lSvrData[iIdx].serv_addr)) < 0) {
			printf("Error bind %s:%d\n",inet_ntoa(lSvrData[iIdx].serv_addr.sin_addr),lSvrData[iIdx].iPortNr);
			printf("server: can't bind local address %s\n",strerror(errno));
			if (lSvrData[iIdx].iFlags & _stlTcpSrvErrExit_) _strSafeExit(_StTlErrTcpSrvBind_);
			return -3;
		}
		lSvrData[iIdx].iFlags |= _LstSrvBound_;
	}

	/* Default staat linger uit op windows
	{
		struct linger LI = {1,2};	// linger for max 2 seconds after close
		//int iLen = sizeof(LI);
		//int iRes = getsockopt(lSvrData[iIdx].sockFd,SOL_SOCKET,SO_LINGER,(char*)&LI,&iLen);
		//int iErr = WSAGetLastError();
		setsockopt(lSvrData[iIdx].sockFd,SOL_SOCKET,SO_LINGER,(char*)&LI,sizeof(LI));
	}
	*/

	// Default to keep-alive control ON
	if ((lSvrData[iIdx].iFlags & _stlTcpSrvNoKeepalive_)==0){
		int kal=-1;
		setsockopt(lSvrData[iIdx].sockFd, SOL_SOCKET,SO_KEEPALIVE, (char*)&kal, sizeof(kal));
	}

	if (lSvrData[iIdx].iFlags & _stlTcpSrvConectPrint_)
		printf("start initializing port %d einde (ok)\n",lSvrData[iIdx].iPortNr);

	// Max 10 connect requests pending.
	listen(lSvrData[iIdx].sockFd,10);
	lSvrData[iIdx].iFlags |= _LstSrvInitOk_;
	return 0;
}

#define _StNotRunning_	0
#define _StStarting_		1
#define _StRunning_			2
#define _StStopping_		3
#define _StStopped_			4

static int _stlSrvStatus=_StNotRunning_;

/* Main listen server
 */
static void _stlSrvMain(void *pUdata)
{
	int newSockFd, clilen;
	struct sockaddr_in cli_addr;
	struct timeval sTimeOut={10,0};
	struct timeval *pTimeOut=&sTimeOut;
	int  i,res,high_f;
	fd_set  readfs;
	_stlSrvStatus=_StRunning_;
	while ( 1 ){
		if (_stlSrvStatus!=_StRunning_) break;
		high_f=-1;
		FD_ZERO(&readfs);
		for (i=0;i<_MaxServers_;i++){
			if (lSvrData[i].iPortNr==0) continue;
			// Check if port is fully initialized
			if ((lSvrData[i].iFlags & _LstSrvInitOk_)==0) _stlTcpSrvInitSocket(i);
			// Check if port needs to be closed
			if (lSvrData[i].iFlags & _LstSrvStopListen_){
				if (lSvrData[i].sockFd>0) stlTcpCloseSocket(lSvrData[i].sockFd);
				printf("stop listening port %d\n",lSvrData[i].iPortNr);
				memset(&lSvrData[i],0,sizeof(lSvrData[i]));
				continue;
			}
			if ((lSvrData[i].iFlags & _LstSrvInitOk_)==0) continue;
			FD_SET(lSvrData[i].sockFd,&readfs); 
			if (lSvrData[i].sockFd>high_f) high_f=lSvrData[i].sockFd;
		}
		if (high_f<0){stlMsWait(1000);continue;}		// No fully initialized sockets for me
		if (pTimeOut){pTimeOut->tv_sec=1;pTimeOut->tv_usec=0;}
		res=select(high_f+1,&readfs, NULL, NULL, pTimeOut);
		if (res==0) continue;												// Timeout heeft plaatsgevonden.
		if (res<0){
			printf("Select error %d %s\nSocket status:\n",res,strerror(errno));
			for (i=0;i<_MaxServers_;i++){
				if (lSvrData[i].iPortNr==0) continue;
				if (lSvrData[i].iFlags & _LstSrvInitOk_){
					printf("poort %d (initializing)\n",lSvrData[i].iPortNr);
				}else{
					printf("poort %d (listening)\n",lSvrData[i].iPortNr);
				}
			}
			stlMsWait(5000);
			continue;
		}
		for (i=0;i<_MaxServers_;i++){
			if (lSvrData[i].iPortNr==0) continue;
			if ((lSvrData[i].iFlags & _LstSrvInitOk_)==0) continue;
			if (FD_ISSET(lSvrData[i].sockFd,&readfs)){
				clilen = sizeof(cli_addr);
				newSockFd = (int)accept(lSvrData[i].sockFd,(struct sockaddr*)&cli_addr,(unsigned int*)(&clilen));
				if (newSockFd < 0) {
					printf("server: accept sock %d len %d\n",lSvrData[i].sockFd,clilen);
					printf("server: accept error %s\n",strerror(errno));
					stlMsWait(5000);
				}else{
					struct stlTcpConInfo *CD;
					CD=malloc(sizeof(struct stlTcpConInfo));
					if (CD==NULL){
						stlTcpCloseSocket(newSockFd);
						printf("Out of memory conserver init %s\n",strerror(errno));
						_strSafeExit(_StTlErrTcpSrvMemory_);
						continue;
					}
					memset(CD,0,sizeof(struct stlTcpConInfo));
					sprintf(CD->srcIp,"%s",inet_ntoa(cli_addr.sin_addr));
					CD->srcPrt  =ntohs(cli_addr.sin_port);
					CD->sockFd  =newSockFd;
					CD->pConfig =lSvrData[i].pConfig;
					CD->lclPrt  =lSvrData[i].iPortNr;

					clilen = sizeof(cli_addr);
					if (getsockname(newSockFd,(struct sockaddr*)&cli_addr,(unsigned int*)(&clilen))==0){
						sprintf(CD->lclIp,"%s",inet_ntoa(cli_addr.sin_addr));
						CD->lclPrt=ntohs(cli_addr.sin_port);
					}

					if (lSvrData[i].iFlags & _stlTcpSrvConectPrint_)
						printf("connect from=%s:%d to %s:%d\n",CD->srcIp,CD->srcPrt,CD->lclIp,CD->lclPrt);
					
					if (lSvrData[i].proces==NULL){
						stlTcpCloseSocket(newSockFd);
						free(CD);
					}else{
						stlThreadStart((void*)lSvrData[i].proces,1,CD,"TcpServerProces");
					}
				}
			}
		}
	}
	for (i=0;i<_MaxServers_;i++){
		if (lSvrData[i].iPortNr==0) continue;
		if (lSvrData[i].sockFd>0) stlTcpCloseSocket(lSvrData[i].sockFd);
		printf("stop listening port %d\n",lSvrData[i].iPortNr);
		memset(&lSvrData[i],0,sizeof(lSvrData[i]));
		continue;
	}
	_stlSrvStatus=_StStopped_;
}
/* Main initialization
 */
static void _stlSrvInit(void)
{
	static stlMutex	mux = stlMutexInitializer;
	if (_stlSrvStatus==_StRunning_) return;
	stlMutexLock(&mux);
	if (_stlSrvStatus==_StNotRunning_){
		_stlSrvStatus=_StStarting_;
		stlTcpLibInit();
		memset(lSvrData,0,sizeof(lSvrData));
		stlThreadStart(_stlSrvMain,1,NULL,"tcp listen server");
	}
	stlMutexUnlock(&mux);
}
static void _stlSrvStop(void)
{
	// Als net aan het opstarten is wacht totdat geheel opgestart
	while (_stlSrvStatus==_StStarting_) stlMsWait(100);

	if (_stlSrvStatus==_StRunning_)
	{
		_stlSrvStatus=_StStopping_;
		while (_stlSrvStatus==_StStopping_) stlMsWait(100);
		_stlSrvStatus=_StNotRunning_;
	}
}



//***************************************************************************************
//**
//**  Public functions
//**
//***************************************************************************************

/* Add a server on a specific TCP port
 *	iPortNr  port number to listen on
 *  pConfig  user pointer to user config data
 *  proces   function pointer to proces new incoming connections (is already separate thread)
 *  iFlags   Som flags to alter the behavior of de server
 * Return 0 Ok
 *       -1 Error 
 */
int stlTcpAddServiceEx(unsigned short iPortNr,void *pConfig,void(*proces)(struct stlTcpConInfo*),int iFlags)
{
	int i;
	_stlSrvInit();
	for (i=0;i<_MaxServers_;i++){
		if (lSvrData[i].iPortNr) continue;
		lSvrData[i].iPortNr =iPortNr;
		lSvrData[i].proces  =proces;
		lSvrData[i].pConfig =pConfig;
		lSvrData[i].iFlags  =iFlags & 0xFFFF;	// Alleen de flags welke mee gegeven mogen worden
		return 0;
	}
	printf("No free server service avail\n");
	return -1; // geen vrije slots meer
}
/* Add a server on a specific TCP port
 *	iPortNr  port number to listen on
 *  proces   function pointer to proces new incoming connections (is already separate thread)
 * Return 0 Ok
 *       -1 Error 
 */
int stlTcpAddService(unsigned short iPortNr,void(*proces)(struct stlTcpConInfo*))
{
	return stlTcpAddServiceEx(iPortNr,NULL,proces,_stlTcpSrvReuseSock_);
}

/* Stop TCP listen thread and removes all listening processes
 * Return 0 Ok;
 */
int stlTcpStopServices(void)
{
	_stlSrvStop();
	return 0;
}

/* Remove TCP server from service pool
 *	iPortNr  Port number we want stopped
 * Return 0 Ok
 *       -1 Invalid port number
 *       -2 Unknown port number
 */
int stlTcpRemoveService(unsigned short iPortNr)
{
	int i;
	if (iPortNr==0) return -1;
	_stlSrvInit();
	for (i=0;i<_MaxServers_;i++){
		if (lSvrData[i].iPortNr!=iPortNr) continue;
		lSvrData[i].pConfig=NULL;
		lSvrData[i].proces =NULL;
		lSvrData[i].iFlags |= _LstSrvStopListen_;
		return 0;
	}
	return -2;
}
/* Get user-data pointer for selected port number
 *	iPortNr port number where service is running
 * Return Data if found
 *        NULL not found
 */
void *stlTcpGetPortUserData(unsigned short iPortNr)
{
	int i;
	for (i=0;i<_MaxServers_;i++){
		if (lSvrData[i].iPortNr==iPortNr) 	return lSvrData[i].pConfig;
	}
	return NULL;
}

/* Get function pointer for selected port number
 *	iPortNr port number where service is running
 * Return function ptr if found
 *        NULL not found
 */
void *stlTcpGetPortUserFunct(unsigned short iPortNr)
{
	int i;
	for (i=0;i<_MaxServers_;i++){
		if (lSvrData[i].iPortNr==iPortNr) 	return lSvrData[i].proces;
	}
	return NULL;
}

