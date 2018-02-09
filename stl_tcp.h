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
#ifndef _TCP_STUF_H_
#define _TCP_STUF_H_

#include <stdio.h>
#include "stl_str.h"
#include "stl_crypt.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

extern int stlTcpDebug;

/* Connect over TCP to IP/port adres
 *	return >0 socket for connection
 *         -1 error create socket
 *         -2 connect error
 */
int stlTcpConnect(const char *HostAddr,unsigned short TcpPort);

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
int stlTcpConnectTout(const char *HostAddr,unsigned short TcpPort,int iTimeoutMs);


/* Convert DNS name to IP adres
**   sDnsName   name to resolve
**   iType      type to resolve (0=normal DNS at the moment)
** Return IP adres of DNS
**        NULL (not found or DNS error)
*/
STP stlTcpDnsResolve(const char *sDnsName,int iType);


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
int stlTcpDnsConnectTout(const char *sDnsName,unsigned short uPort,int iTimeoutMs);

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
int stlTcpDnsConnect(const char *sDnsName,unsigned short uPort);

/* Test if IP number is valid IP number
**  sIpNumber  ip number to test
** Return 1=Valid 
**        0=Invalid
*/
int stlTcpTestValidIp(const char *sIpNumber);

/* Close socket
 */
void stlTcpCloseSocket(int iSockFd);

/* Turn socket blocking mode on(1) or off(0)
 *	return 0 Ok
 */ 
int stlTcpBlock(int sockFd,int onFlg);

/* set socket to send keep-alive packets at regular intervals
*   sockFd  socket to perform keep-alive on
*  return 0 Ok
*         1 Error
*/
int stlTcpKeepAlive(int sockFd);

/* set socket to send keep-alive packets at regular intervals
*   sockFd  socket to perform keep-alive on
*   aantal  bij windows is dit altijd 10 is niet aan te passen.
*   interval  tijd tussen de keep alive verzoeken.
*  return 0 Ok
*         1 Error
*/
int stlTcpKeepAliveParameters(int sockFd,int aantal,int interval);

/* set / clear socket state to send immediately after write commando. (don't buffer(nagle) data)
*   sockFd  socket to set no-delay option on
*   onFlg   1 enable no-delay 0 disable
*  return 0 Ok
*         1 Error
*/
int stlTcpNoDelay(int sockFd,int onFlg);

#define _BufSize_		20480
#define _MagicTcp_	0x5A12A521

typedef struct stlTcpConn{
	unsigned char rBuf[_BufSize_];	// Receive buffer
	unsigned char xBuf[_BufSize_];	// Transmit buffer
	int  rLen,rOfs;				// Lengte en offset.
	int  xOfs;					// Zend offset
	int  sockFd;				// Socket
	int  waitTime;				// Aantal ms wachten tussen retry. (100 ms default)
	int  magic;					// Magic kooky
	int  iMaxBps;				// Max transfer speed (0=Unlimited)
	STP  sLine;					// STP struct to current read line
	stlMutex muxTX;				// Transmit mutex (zodat concurrent write commando's niet door elkaar heen komen)
	PSTLCRYPT pRxCrypt;	// Encryption data receive   (must be created separately)
	PSTLCRYPT pTxCrypt;	// Encryption data transmit  (must be created separately)
} stlTcpConn;


/* Start data encryption on TCP data stream.
**  stc       TCP manager structure pointer
**  pRxCrypt  (optional) encryption data for received data
**  pTxCrypt  (optional) encryption data for transmitted data
*/
void stlTcpCryptStart(stlTcpConn *stc,PSTLCRYPT pRxCrypt,PSTLCRYPT pTxCrypt);

/* Initialiseer socket structure
 *	sockFd (socket file handle to use)
 * return: socket structure
 */
struct stlTcpConn *stlTcpInit(int sockFd);


/* Release and close socket structure
 *	stc pointer allocated from stlTcpInit
 */
int stlTcpRelease(struct stlTcpConn *stc);

/* Release but don't close socket structure
 *	stc pointer allocated from stlTcpInit
 */
int stlTcpReleaseNoClose(struct stlTcpConn *stc);

/* Clear local send en receive buffer of TCP connection
 *       This dos not clear the kernel buffers of the stream.
 * stc pointer to communication struct	
 */
void stlTcpClear(struct stlTcpConn *stc);

/* Get character from TCP data
 *	stc     communication structure pointer
 *  timeout timeout in ms (when <0 wil wait forever)
 * Return >=0 character read from stream
 *         -1 NULL TCP struct error
 *         -2 Invalid TCP struct error
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Connection lost
 */
int stlTcpGetChar(struct stlTcpConn *stc,int timeout);

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
 *        -99 Connection lost
 */
int stlTcpReadMax(struct stlTcpConn *stc,void *buf,int iLen,int timeout1,int timeout2,const char *sIgnore,const char *sStop,int *iReadCnt);

/* Read max number of bytes
 *	stc      communication structure
 *  buf      buffer to write received chars in
 *  iLen     max number of bytes to receive
 *  timeout  time to wait for first character to arrive      (<0 wait indefinitely)
 *  iReadCnt pointer to integer where number of read bytes is stored. (optional)
 * return:  0 Max number of bytes read
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 timeout
 *         -5 NULL receive buffer
 *        -99 Connection lost
 */
int stlTcpRead(struct stlTcpConn *stc,void *buf,int iLen,int timeout,int *iReadCnt);

/* Read string struct from TCP data stream
 *	stc      communication structure
 *  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
 *  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
 *  sIgnore  string of characters to ignore in data stream.           (optional)
 *  sStop    string of chars to stop on.                              (optional)
 * return: NULL timeout or read error
 *         Data data was read til stop char was reached
 */
STP stlTcpReadLineEx(struct stlTcpConn *stc,int timeout1,int timeout2,const char *sIgnore,const char *sStop);

/* Read string struct from TCP data stream
 *	stc      communication structure
 *  timeout  time to wait for characters to arrive (resets after every char)(<0 wait indefinitely)
 * return: NULL timeout or read error
 *         Data line of chars was rad from TCP stream
 */
STP stlTcpReadLine(struct stlTcpConn *stc,int timeout);

/************************************************************************/
/*  W R I T E    F U N C T I O N S                                      */
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
int stlTcpFlush(struct stlTcpConn *stc,int timeout);

/* Write one char to TCP connection
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpPutChar(struct stlTcpConn *stc,char ch,int timeout);

/* Write block of data to TCP connection (Flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWrite(struct stlTcpConn *stc,const unsigned char *buf,int len,int timeout);

/* Write TCP connection like printf (Flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWritef(struct stlTcpConn *stc,int timeout,const char *fmt,...);

/* Write block of data to TCP connection (don't flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWriteNF(struct stlTcpConn *stc,unsigned char *buf,int len,int timeout);

/* Write TCP connection like printf (don't flush after last character)
 * Return   0 Ok
 *         -1 NULL struct pointer 
 *         -2 Invalid struct pointer
 *         -3 Timeout
 *         -4 Unknown error
 *        -99 Not connected (or connection lost)
 */
int stlTcpWriteNFf(struct stlTcpConn *stc,int timeout,const char *fmt,...);

/* Connect 2 sockets together with optional logging capabilities
 *  iSock1  First socket to use
 *  iSock2  Second socket to use
 *  iByteUp (optional) bytes from iSock1 -> iSock2
 *  iByteDn (optional) bytes from iSock2 -> iSock1 (may be same as iByteUp)
 *  fUp     (optional) FILE to write data from iSock1 -> iSock2
 *  fDn     (optional) FILE to write data from iSock2 -> iSock1 (may be same as fUp)
 */
void stlTcpConnectSocketsLog(int iSock1,int iSock2,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn);

/* Connect 2 sockets together with optional logging capabilities
**  iSock1  First socket to use
**  iSock2  Second socket to use
**  iByteUp (optional) bytes from iSock1 -> iSock2
**  iByteDn (optional) bytes from iSock2 -> iSock1 (may be same as iByteUp)
**  fUp     (optional) FILE to write data from iSock1 -> iSock2
**  fDn     (optional) FILE to write data from iSock2 -> iSock1 (may be same as fUp)
**  pRxCry1 (optional) decryption for data from iSock1 (must be created separately)
**  pTxCry1 (optional) encryption for data to   iSock1 (must be created separately)
**  pRxCry2 (optional) decryption for data from iSock2 (must be created separately)
**  pTxCry2 (optional) encryption for data to   iSock2 (must be created separately)
*/
void stlTcpConnectSocketsLogCrypt(int iSock1,int iSock2,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn,PSTLCRYPT pRxCry1,PSTLCRYPT pTxCry1,PSTLCRYPT pRxCry2,PSTLCRYPT pTxCry2);

extern int connectSocketTimeoutSec;

/* Connect 2  TCP connections together
 *	stc1    First connection
 *  stc2    Second connection
 *  iByteUp (optional) bytes from stc1 -> stc2
 *  iByteDn (optional) bytes from stc2 -> stc1 (may be same as iByteUp)
 *  fUp     (optional) FILE to write data from stc1 -> stc2
 *  fDn     (optional) FILE to write data from stc2 -> stc1 (may be same as fUp)
 */
void stlTcpConnectConsLog(struct stlTcpConn *stca,struct stlTcpConn *stcb,int *iByteUp,int *iByteDown,FILE *fUp,FILE *fDn);

/* Connect 2 TCP connections together
 *	stc1    First connection
 *  stc2    Second connection
 */
void stlTcpConnectCons(struct stlTcpConn *stca,struct stlTcpConn *stcb);


/************************************************************************/
/*  Multi threaded TCP server functions and definitions                   */
/************************************************************************/

#define _stlTcpSrvErrExit_      0x00001         // Abort application when socket not availeble
#define _stlTcpSrvNoKeepalive_	0x00002					// dont send TCP keep alive handshake
#define _stlTcpSrvConectPrint_  0x00004         // Print connect info when new connections are made
#define _stlTcpSrvReuseSock_    0x00008         // reuse socket port numbers

struct stlTcpConInfo{
        int  srcPrt;    // Port number where connection originates from
        char srcIp[20]; // IP Adres where connection originates from
        int  lclPrt;    // My own port number
        char lclIp[20]; // My own IP adres
        int  sockFd;    // Socket for this connection
        void *pConfig;  // User config pointer
};

/* Add a server on a specific TCP port
 *	iPortNr  port number to listen on
 *  pConfig  user pointer to user config data
 *  proces   function pointer to proces new incoming connections (is already separate thread)
 *  iFlags   Som flags to alter the behavior of de server
 * Return 0 Ok
 *       -1 Error 
 */
int stlTcpAddServiceEx(unsigned short iPortNr,void *pConfig,void(*proces)(struct stlTcpConInfo*),int iFlags);

/* Add a server on a specific TCP port
 *	iPortNr  port number to listen on
 *  proces   function pointer to proces new incoming connections (is already separate thread)
 * Return 0 Ok
 *       -1 Error 
 */
int stlTcpAddService(unsigned short iPortNr,void(*proces)(struct stlTcpConInfo*));

/* Remove TCP server from service pool
 *	iPortNr  Port number we want stopped
 * Return 0 Ok
 *       -1 Invalid port number
 *       -2 Unknown port number
 */
int stlTcpRemoveService(unsigned short iPortNr);

/* Stop TCP listen thread and removes all listening processes
 * Return 0 Ok;
 */
int stlTcpStopServices(void);

/* Get user data pointer for selected port number
 *	iPortNr port number where service is running
 * Return Data if found
 *        NULL not found
 */
void *stlTcpGetPortUserData(unsigned short iPortNr);

/* Get function pointer for selected port number
 *	iPortNr port number where service is running
 * Return function ptr if found
 *        NULL not found
 */
void *stlTcpGetPortUserFunct(unsigned short iPortNr);


/************************************************************************/
/* UDP functions                                                        */
/************************************************************************/

typedef struct stlUdpConnStruct{
	char sRemoteHost[20];		// Host IP om data naartoe te zenden.
	unsigned short uRxPort;		// UDP poort waar we op luisteren 0 system allocated
	unsigned short uTxPort;		// UPP poort waar data naartoe gestuurd moet worden
	int  sockFd;				// Socket
	int  magic;					// Magic kooky
} stlUdpConn;

/* Close UDP port and release its resources
*/
void stlUdpRelease(stlUdpConn *suc);

/* Create a new UDP connection
**   RemoteAddr    (opt) remote IP to send the data to.
**   UdpRemotePort (opt) remote UDP to send the data to.
**   UdpLocalPort  (opt) local UDP port to listen on.
** Return NULL error
**        ptr  struct to send and receive data on.
*/
stlUdpConn * stlUdpConnectIp(const char *RemoteAddr,unsigned short UdpRemotePort,unsigned short UdpLocalPort);
stlUdpConn * stlUdpConnect(void);

/* Bind UDP socket aan specifiek netwerk kaart optioneel ook multi cast group adres
*/
int stlUdpBind(stlUdpConn *suc,const char *LocalAddr,unsigned short UdpLocalPort);
int stlUdpBindMC(stlUdpConn *suc,const char *LocalAddr,unsigned short UdpLocalPort,const char *McastAddr);

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
int stlUdpSendEx (stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,STP sData);
int stlUdpSendExv(stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,const void *pData,int iDataLen);
int stlUdpSendExf(stlUdpConn *suc,const char *sRemoteIP, unsigned short uRemotePort,char *fmt,...);
int stlUdpSend   (stlUdpConn *suc,STP sData);
int stlUdpSendf  (stlUdpConn *suc,const char *fmt,...);

/* Receive data from UDP socket
**    suc         struct from stlUdpConnect
**    iMsTimeout  time in ms to wait for data <0 wait unlimited
**    piErr       (opt) error number in case of NULL return
** Return Data    Data received from UDP port
**        NULL    in case of timeout (*piErr=4) or error (*piErr>0)
*/
STP stlUdpRecv(stlUdpConn *suc,int iMsTimeout,int *piErr);

/* Start UDP server (only returns in case of error)
**    suc         struct from stlUdpConnect
**    UdpCallback Callback function when data on UDP is received.
**    pUserData   user defined data witch is forwarded to the callback function
** Return -1 NULL stlUdpConn data
**        -2 Corrupted stlUdpCon data
**        -3 Unconnected stlUdpCon data
**        -4 No local UDP port defined
*/
int stlUdpServer(stlUdpConn * suc,void (*UdpCallback)(stlUdpConn * suc,STP sRxData,void *pUserData),void *pUserData);

/************************************************************************/
/*                                                                      */
/************************************************************************/


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
int stlSmtpSendEx(const char *sHostName,const char *sUname,const char *sPassw,const char *sFrom,STP sDest,STP sEml,STP *sDiag);

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
int stlSmtpSend(const char *sHostName,const char *sFrom,STP sDest,STP sEml);

/* Create plain text e-mail message
**   sSubject    subject of e-mail message
**   sFromName   Full name of sender
**   sFromEmail  e-mail adres of sender
**   sToName     Full name of recipient
**   sToEmail    e-mail adres of recipient
**   sMessage    message to send 
** Return compiled e-mail message
*/
STP stlEmailCreate(char *sSubject,char *sFromName,char *sFromEmail,char *sToName,char *sToEmail,char *sMessage);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

