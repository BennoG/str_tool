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
#ifndef _STL_RS232_H_
#define _STL_RS232_H_

#include "stl_str.h"

#ifdef  __cplusplus
extern "C" {
#endif


/* Init serial communication simulation function
 *	sPort     name of port to simulate
 *  simRdWrd  Function to be called on reading and writing to serial port
 *  pUser     User definable data (passed on to simRdWrd)
 * Return 0 Ok
 *       -1 Error sim table full (too many simulation ports used)
 */
int stlSerialSimAdd(char *sPort,int(*simRdWrd)(int iWrFg,char cChr,void *pUsr),void *pUsr);


/* Open com poort
 *	sPort naam van poort COM1-COM99 of ttyS0-ttyS99
 * Return 0  error 
 *        >  handle (socket naar poort)
 */
int stlSerialOpen(const char *sPort);

/* Open com poort
**	sPort naam van poort COM1-COM99 of ttyS0-ttyS99
**  baud   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
**  parity parity bits (N,O,E) none,odd,even
**  data   data bits (6,7,8)
**  stop   stop bits  (1,2)
** Return 0  error 
**        >  handle (socket naar poort)
*/
int stlSerialOpenEx(const char *sPort,int baud,int parity,int databits,int stopbits);


/* close com poort
 *	iHandle  handle of com poort
 * Return <0 error
 *         0 Ok
 */
int stlSerialClose(int iHandle);

/* Set baudrate of serial port
 *	iHandle  socket (handel) to serial port
 *  baud   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
 *  parity parity bits (N,O,E) none,odd,even
 *  data   data bits (6,7,8)
 *  stop   stop bits  (1,2)
 * Return 0 ok
 *       -1 can not get serial params
 *       -2 can not set serial params
 *       -3 illegal socket number
 */
int stlSerialSetParams(int iHandle,int baud,int parity,int databits,int stopbits);

/* Read data from serial port
 *	iHandle  handle naar serial port
 *  buf    buffer to read data in
 *  imax   max number of chars to read.
 * Return >=0 number of bytes read
 *        -1  Illegal handle
 *        -2  Read error (illegal handle ??)
 */
int stlSerialRead(int iHandle,char *buf,int imax);

/* Write data to serial port
 *	iHandle  handle naar serial port
 *  buf    buffer of data to write
 *  iLen   number of bytes to write
 * Return >=0  number of bytes written
 *        -1  Illegal handle
 *        -2  Write error (illegal handle ??)
 */
int stlSerialWrite(int iHandle,const char *buf,int iLen);

/* Write single char to serial port
**	iHandle  handle naar serial port
**  ch       char to write to port
** Return <=0 error
**         >0  number of bytes written
*/
int stlSerialPutch(int iHandle,char ch);

// TODO:
int stlSerialHwFlow(int iHandle,int iOn);	// enable disable (RTS/CTS) flow control


typedef struct stlSerialStrut
{
	int iHdl;
	char sBuf[250];
	int iBufLen;	// Aantal bytes in buffer
	int iBufOfs;	// huidige offset in buffer
	int iMagic;
	STP sLine;		// ontvangst data voor readline
	stlMutex mux;
}stlSerial;

/* Initialiseer serial buffered reader
**  iHdl  handle to serial port fd
** Return NULL error
**        stlSerial pointer
**/
stlSerial * stlSerialInit(int iHdl);

/* Lock a connection for private use in al multi threaded program.
*/
void stlSerialLock(stlSerial *sl);

/* Unlock a connection for private use in al multi threaded program.
*/
void stlSerialUnlock(stlSerial *sl);

/* Release serial buffer read data
** Also closes the serial port
**/
void stlSerialRelease(stlSerial *sl);

/* Read 1 character from serial port
**  sl        pointer to serial buffer struct
**  iTimeout  Read timeout in ms (-1 wait forever)
** Return 0 ok data read
** 
**/
int stlSerialGetch(stlSerial *sl,int iTimeOutMSec);

/* Clear alle characters in read buffer of serial line
**  sl        pointer to serial buffer struct
** Return 0 ok data read
**       -3 invalid buffer struct
**/
int stlSerialRxClear(stlSerial *sl);

/* Write string to serial port (like printf)
**  iHdl  handle to serial port fd
**  fmt   format string like printf
** Return >=0  number of bytes written
**        -1  Illegal handle
**        -2  Write error (illegal handle ??)
**/
int stlSerialWritef(int iHdl,char *fmt,...);

/* Read max number of bytes
**	sl       communication structure
**  buf      buffer to write received chars in
**  iLen     max number of bytes to receive
**  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
**  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
**  sIgnore  string of characters to ignore in data stream.           (optional)
**  sStop    string of chars to stop on.                              (optional)
**  iReadCnt pointer to integer where number of read bytes is stored. (optional)
** return: >0 sStop character hit
**          0 Max number of bytes read
**         -1 NULL struct pointer 
**         -2 Invalid struct pointer
**         -3 NULL receive buffer
*/
int stlSerialReadMax(stlSerial *sl,void *buf,int iLen,int timeout1,int timeout2,const char *sIgnore,const char *sStop,int *iReadCnt);

/* Read string struct from serial connection stream
**	sl      communication structure
**  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
**  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
**  sIgnore  string of characters to ignore in data stream.           (optional)
**  sStop    string of chars to stop on.                              (optional)
** return: NULL timeout or read error
**         Data data was read til stop char was reached
*/
STP stlSerialReadLineEx(stlSerial *sl,int timeout1,int timeout2,const char *sIgnore,const char *sStop);

/* Read string struct from serial connection stream
**	sl       communication structure
**  timeout  time to wait for characters to arrive (resets after every char)(<0 wait indefinitely)
** return: NULL timeout or read error
**         Data line of chars was rad from serial port stream
*/
STP stlSerialReadLine(stlSerial *sl,int timeout);


#ifdef  __cplusplus
}
#endif


#ifdef  __cplusplus

namespace ansStl
{
	class rs232
	{
	private:
		void initVars(){iHdl = 0; sl = NULL; bSerCleanup = true;}
		int iHdl;
		stlSerial *sl;
		bool bSerCleanup;
	public:
		/*  portName (COM1 .. COM99 of ttyS0 ... ttyS99)
		**  baud   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
		**  parity parity bits (N,O,E) none,odd,even
		**  data   data bits (6,7,8)
		**  stop   stop bits  (1,2)
		*/
		rs232(stlSerial *ser);
		rs232(const char *portName,int baud = 9600,int parity = 'n' ,int databits = 8,int stopbits = 1);
		rs232(int portNr,int baud = 9600,int parity = 'n' ,int databits = 8,int stopbits = 1);
		~rs232();
		/* writing to the serial port */
		int write(const char *buf         ,int bufLen = -1);
		int write(const unsigned char *buf,int bufLen = -1);
		int write(STP buf);
		int write(cST &buf);
		int write(cST *buf);
		int writef(const char *buf,...);
		/* reading from the serial port */
		STP readLine(int timeout1 = -1,int timeout2 = -1,const char *sIgnore = "\r" ,const char *sStop = "\n");
		int read(void *buf,int iLen,int timeout1 = -1,int timeout2 = -1,const char *sIgnore = NULL,const char *sStop = NULL,int *iReadCnt = NULL);
		STP read(int timeout1 = -1,int timeout2 = -1);
		int getc(int timeout = -1);
		STP readLen(int numBytes,int timeout1 = -1,int timeout2 = -1);
	};
}

#endif

#endif
