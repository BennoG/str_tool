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
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#  include <windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#  include <termios.h>
#  include <unistd.h>
#  include <stdlib.h>
#  include <unistd.h>
#  include <errno.h>
#  include <sys/ioctl.h>
#  include <sys/time.h>
#  include <fcntl.h>
#endif

/* Moxa UPort 1450
** ln -s ttyMXUSB0 ttyS10
** ln -s ttyMXUSB1 ttyS11
** ln -s ttyMXUSB2 ttyS12
** ln -s ttyMXUSB3 ttyS13
*/

#include "stl_str.h"
#include "stl_thread.h"
#include "stl_rs232.h"
#include "internal.h"

using namespace ansStl;

/************************************************************************/
/* Simulation of serial device                                          */
/************************************************************************/


struct serSimRedir{
	int  iFlg;
	char sName[15];				// name of poort COM1 - COM9 ttyS0-ttyS99  ttyMXUSB0-99 (moxa USB)
	void *pUser;				// user data pointer.
	int(*simRdWrd)(int iWrFg,char cChr,void *pUsr);
};

#define _MaxSim_	5
#define _SimHdlBase_	2500
static struct serSimRedir _serSim[_MaxSim_];
/* Initialize structure
 */
static void _serSimInit(void)
{
	static int iDone=0;
	if (iDone) return;
	iDone++;
	memset(_serSim,0,sizeof(_serSim));
}
/* (internal) Test if port needs to be simulated
 *	sPort name of poort COM1 - COM9 ttyS0-ttyS99
 * Return 0 (geen simulatie noodzakelijk)
 *       >0 handle to be used for simulation
 */
static int _serSimTest(const char *sPort)
{
	int i;
	_serSimInit();
	for (i=0;i<_MaxSim_;i++){
		if (_serSim[i].iFlg==0) continue;
		if (stricmp(_serSim[i].sName,sPort)==0){
			return (_SimHdlBase_+i);
		}
	}
	return 0;
}
/* (internal) Read data from simulated serial port
 *	iHandle handle assigned to simulated port
 *  buf     buffer to store data in
 *  iMax    max number of chars to read
 * Return   number of bytes read.
 */
static int _serSimRead(int iHandle,char *buf,int iMax)
{
	int i,ch;
	iHandle-=_SimHdlBase_;
	if ((iHandle<0)||(iHandle>=_MaxSim_)) return 0;
	if (_serSim[iHandle].simRdWrd==NULL) return 0;
	for (i=0;i<iMax;i++){
		ch=_serSim[iHandle].simRdWrd(0,0,_serSim[iHandle].pUser);
		if (ch<0) break;
		buf[i]=ch;
	}
	return i;
}
/* (internal) Write data to simulated serial port
 *	iHandle handle assigned to simulated port
 *  buf     buffer with data to be send
 *  iBytes  number of bytes to write
 * Return   number of bytes written.
 */
static int _serSimWrite(int iHandle,const char *buf,int iBytes)
{
	int i,ch;
	iHandle-=_SimHdlBase_;
	if ((iHandle<0)||(iHandle>=_MaxSim_)) return 0;
	if (_serSim[iHandle].simRdWrd==NULL) return 0;
	for (i=0;i<iBytes;i++){
		ch=_serSim[iHandle].simRdWrd(0,buf[i],_serSim[iHandle].pUser);
		if (ch<0) return i;
	}
	return iBytes;
}

/* Init seraial communication simulation function
 *	sPort     name of port to simulate
 *  simRdWrd  Function to be called on reading and writing to serial port
 *  pUser     User definable data (passed on to simRdWrd)
 * Return 0 Ok
 *       -1 Error sim table full (too many simulation ports used)
 */
int stlSerialSimAdd(char *sPort,int(*simRdWrd)(int iWrFg,char cChr,void *pUsr),void *pUsr)
{
	static stlMutex mux=stlMutexInitializer;
	int i;
	_serSimInit();
	stlMutexLock(&mux);
	for (i=0;i<_MaxSim_;i++){
		if (_serSim[i].iFlg) continue;
		strncpy(_serSim[i].sName,sPort,sizeof(_serSim[i].sName)-1);
		_serSim[i].simRdWrd=simRdWrd;
		_serSim[i].pUser   =pUsr;
		_serSim[i].iFlg    =1;
		stlMutexUnlock(&mux);
		return 0;
	}
	stlMutexUnlock(&mux);
	return -1;
}


#ifdef _WIN32

/************************************************************************/
/* Windows support files                                                */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/* close com poort
 *	iHandle  handle of com poort
 * Return <0 error
 *         0 Ok
 */
int stlSerialClose(int iHandle)
{
	HANDLE hComm=(HANDLE)iHandle;
	if (hComm<=0) return -1;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return 0;
	CloseHandle(hComm);
	return 0;
}
static void _stlSerialErrorPrint(void) 
{ 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	printf("Error (%X)%s\n",dw,lpMsgBuf);
	LocalFree(lpMsgBuf);
}

/* Open com poort
 *	sPort naam van poort COM1-COM99 of ttyS0-ttyS99
 * Return 0  error 
 *        >  handle (socket naar poort)
 */
int stlSerialOpen(const char *sPort)
{
	char devname[50];
	int res;
	HANDLE hComm;
	COMMTIMEOUTS ComTime;

	hComm=(HANDLE)_serSimTest(sPort);
	if (hComm) return (int)hComm;

	res=-1;
	if (strnicmp(sPort,"COM",3)==0){
		res=atoi(sPort+3);
	}
	if (strnicmp(sPort,"ttys",4)==0){
		res=atoi(sPort+4)+1;
	}
	if (res<=0) return 0;
	//sprintf(devname,"COM%d",res);	
	sprintf(devname,"\\\\.\\COM%d",res);	// deze werkt wel met COM10 en hoger

	//********************
	//** Serial communications.
	hComm = CreateFile(devname,GENERIC_READ | GENERIC_WRITE, 
						0, 
						0, 
						OPEN_EXISTING,
						0,  // FILE_FLAG_OVERLAPPED,
						0);
	if (hComm == INVALID_HANDLE_VALUE){
		_stlSerialErrorPrint();
		return 0;
	}

	memset(&ComTime,0,sizeof(ComTime));
	//ComTime.ReadIntervalTimeout        =1;
	//ComTime.ReadTotalTimeoutConstant   =1;
	//ComTime.ReadTotalTimeoutMultiplier =1;
	ComTime.ReadIntervalTimeout        =MAXDWORD;
	ComTime.ReadTotalTimeoutConstant   =0;
	ComTime.ReadTotalTimeoutMultiplier =0;
	ComTime.WriteTotalTimeoutConstant  =1;
	ComTime.WriteTotalTimeoutMultiplier=1;
	SetCommTimeouts(hComm,&ComTime);
	return (int)hComm;
}



static struct { int ibaud,speed; } btable[] = {
		{ 115200,	CBR_115200 },
		{ 57600,	CBR_57600 },
		{ 38400,	CBR_38400 },
		{ 19200,	CBR_19200 },
		{ 9600,		CBR_9600 },
		{ 4800, 	CBR_4800 },
		{ 2400,		CBR_2400 },
		{ 1200,		CBR_1200 },
		{ 600,		CBR_600 },
		{ 300,		CBR_300 },
		{ 110,		CBR_110 },
		{ 0,        0}
};


/* Set baudrate of serial port
 *	iHandle  socket (handel) to serial port
 *  buad   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
 *  parity parity bits (N,O,E) none,odd,even
 *  data   data bists (6,7,8)
 *  stop   stop bits  (1,2)
 * Return 0 ok
 *       -1 can not get serial params
 *       -2 can not set serial params
 *       -3 illegal socket number
 */
int stlSerialSetParams(int iHandle,int baud,int parity,int databits,int stopbits)
{
	HANDLE hComm=(HANDLE)iHandle;
	int i;
	DCB comDcb;
	if (hComm==NULL) return -3;

	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return 0;

	memset(&comDcb,0,sizeof(comDcb));
	
	if (!GetCommState(hComm, &comDcb)) return -1;

	for (i = 0; btable[i].ibaud; i++){
		if (baud == btable[i].ibaud){
			comDcb.BaudRate = btable[i].speed;			// Set new state.
			break;
		}
	}
	switch (parity){
	case 'o':
	case 'O':
	case  0 :	comDcb.Parity = ODDPARITY;  break;
	case 'e':
	case 'E':
	case  1 :	comDcb.Parity = EVENPARITY; break;
	default:	comDcb.Parity = NOPARITY;   break;
	}
	if ((databits>5)&&(databits<9))	comDcb.ByteSize = databits;
	switch (stopbits){
	case 1:		comDcb.StopBits = ONESTOPBIT; break;
	case 2:		comDcb.StopBits = TWOSTOPBITS;break;
	}

	if (!SetCommState(hComm, &comDcb)) return -2;
	return 0;
}

/* Set hardware flow control on / off
**  iHandle  handle naar serial port
**  iOn      Hardware flow on / off
**/
int stlSerialHwFlow(int iHandle,int iOn)
{
	//return _stlSerialCrtsRts(iHandle,iOn);
	return iOn;
}

/* Read data from serial port
 *	iHandle  handle naar serial port
 *  buf    buffer to read data in
 *  imax   max numver of chars to read.
 */
int stlSerialRead(int iHandle,char *buf,int imax)
{
	HANDLE hComm=(HANDLE)iHandle;
	DWORD numRd=0;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return _serSimRead(iHandle,buf,imax);
	if (hComm<=0) return -1;
	ReadFile(hComm,buf,imax,&numRd,NULL);
	return numRd;
}

/* Write data to serial port
 *	iHandle  handle naar serial port
 *  buf    buffer of data to write
 *  iLen   number of bytes to write
 * Return <=0 error
 *        >0  number of bytes written
 */
int stlSerialWrite(int iHandle,const char *buf,int iLen)
{
	HANDLE hComm=(HANDLE)iHandle;
	DWORD numWr=0;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return _serSimWrite(iHandle,buf,iLen);
	if (hComm<=0) return -1;
	WriteFile(hComm,buf,iLen,&numWr,NULL);
	return numWr;
}

/* Write single char to serial port
**	iHandle  handle naar serial port
**  ch       char to write to port
** Return <=0 error
**         >0  number of bytes written
*/
int stlSerialPutch(int iHandle,char ch)
{
	HANDLE hComm=(HANDLE)iHandle;
	DWORD numWr=0;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return _serSimWrite(iHandle,&ch,1);
	if (hComm<=0) return -1;
	WriteFile(hComm,&ch,1,&numWr,NULL);
	return numWr;
}



#endif

#if defined(__linux__) || defined(__APPLE__)

#define _MaxFd_ 25

static int devFdTab[_MaxFd_];
static int devDevNr[_MaxFd_];
static char devDevName[_MaxFd_][50];
static int devFdInit=0;

static void stlSerialDevHdlBind(int iDevNr,int devFd,char *devName)
{
    int i;
    if (devFdInit==0){
        memset(devFdTab  ,0,sizeof(devFdTab  ));
        memset(devDevNr  ,0,sizeof(devDevNr  ));
		memset(devDevName,0,sizeof(devDevName));
        devFdInit=1;
    }
    for (i=0;i<_MaxFd_;i++)
    {
		if (devFdTab[i]==0)
		{
			devFdTab[i]=devFd;
			devDevNr[i]=iDevNr;
			strncpy(devDevName[i],devName,sizeof(devDevName[i])-1);
			return;
		}
    }
    printf("stl serial device handle matchtable full\n");
    return;
}
static int stlSerialDevHdlIdx(int iHandle)
{
	int i;
	for (i=0;i<_MaxFd_;i++)
		if (devFdTab[i]==iHandle) return i;
	return -1;
}



static int stlSerialDevLock(char *dev)
{
	FILE *f1;
	char lockName[50];
	char devName[50];
	
	strcpy(devName,dev);
	memcpy(devName,"LCK..",5);
	strcpy(lockName,"/var/lock/");
	strcat(lockName,devName);

//	sprintf(lockName,"/var/lock/LCK..ttyS%d",iDevNr);
	f1=fopen(lockName,"rb");
	if (f1){
		// er is een lock voor deze poort
		int pid;
		STP sLin=stlFileReadLine(f1);
		fclose(f1);
		pid=atoi(sLin?sLin->sBuf:"0");
		stlFree(sLin);
		// Controleer of deze lock nog aan een levend pid hangt
		sLin=stlSetStf("/proc/%d/exe",pid);
		f1=fopen(sLin->sBuf,"rb");
		stlFree(sLin);
		if (f1){
			// Ja poort is gelocked abort lock
			fclose(f1);
			printf("modem %s is gelocked door pid %d\n",dev,pid);
			return 1;
		}
	}
	f1=fopen(lockName,"wb");
	if (f1){
		fprintf(f1,"%d\n",getpid());
		fclose(f1);
		return 0;
	}
	return 1;
}

static int stlSerialDevUnlock(char *dev)
{
	char lockName[50];
	char devName[50];
	strcpy(devName,dev);
	memcpy(devName,"LCK..",5);
	strcpy(lockName,"/var/lock/");
	strcat(lockName,devName);
	unlink(lockName);
	return 0;
}
static int stlSerialDevUnlockHdl(int iHandle)
{
    int i;
    if (devFdInit==0) return -1;
    for (i=0;i<_MaxFd_;i++)
    {
		if (devFdTab[i]==iHandle)
		{
			stlSerialDevUnlock(devDevName[i]);
			devFdTab[i]=0;
			devDevNr[i]=0;
			devDevName[i][0]=0;
			return 0;
		}
    }
    return -2;
}


static int _stlSerialRaw (int fd) 
{
	struct termios tio;
	if (tcgetattr (fd, &tio)<0) return -1;
	
	tio.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
	tio.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT);
	tio.c_oflag &= ~OPOST;
	tio.c_cflag |= CS8;
	tio.c_cc[VMIN]  = 1;
	tio.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSANOW, &tio) < 0) return -1;	
	return 0; 
}
/* Enable / Disable hardware handshake
 */
static int _stlSerialCrtsRts (int fd, int crtscts)
{
	struct termios tio;
	if (tcgetattr(fd, &tio) < 0)	return -1;
	if (crtscts)	tio.c_cflag |= CRTSCTS;
	else			tio.c_cflag &= ~CRTSCTS;
	if (tcsetattr(fd, TCSANOW, &tio) < 0)	return -1;

	// onderstaande lijkt voor USB->RS232 dongels noodzakelijk te zijn.
	// dit is nodig omdat op suse 10.3 op de een of ander manier
	// de instellingen van stlSerialHwFlow(iHdl,0) niet aktief worden gemaakt.
	// hierdoor blijft hardware flow aanstaan en word er dus nooit data verzonden
	// dit gebeurd alleen bij USB devices
	if (crtscts==0)
	{
		int iNew;
		int iCtr = fcntl(fd, F_GETFL, 0);
		int iIdx = stlSerialDevHdlIdx(fd);
		if ((iIdx>=0)&&(strnicmp(devDevName[iIdx],"/dev/ttyUSB",11)==0))
		{
			close(fd);
			iNew = open(devDevName[iIdx],iCtr);
			if (iNew!=fd){fd=dup2(iNew,fd); close(iNew);}
		}
	}
	return 0;
} 

int stlSerialHwFlow(int iHandle,int iOn)
{
	return _stlSerialCrtsRts(iHandle,iOn);
}
/* Set the O_NONBLOCK flag of desc if value is nonzero,
   or clear the flag if value is 0.
   Return 0 on success, or -1 on error with errno set. */ 
static int _stlSerialSetBlock(int desc, int value)
{
    int oldflags = fcntl (desc, F_GETFL, 0);
    /* If reading the flags failed, return error indication now. */
    if (oldflags < 0)
	return oldflags;
    /* Set just the flag we want to set. */
    if (value == 0)
	oldflags |= O_NONBLOCK;
    else
	oldflags &= ~O_NONBLOCK;
    /* Store modified flag word in the descriptor. */
    return fcntl (desc, F_SETFL, oldflags);
}



/* Open com poort
 *	sPort naam van poort COM1-COM99 of ttyS0-ttyS99 ttyUSB0-ttyUSB99
 * Return 0  error 
 *        >  handle (socket naar poort)
 */
int stlSerialOpen(const char *sPort)
{
	char devname[50];
	int res,devFd;

	// serial simulation suppprt
	res=_serSimTest(sPort);
	if (res) return res;

	res=-1;
	if (strnicmp(sPort,"COM",3)==0){
		res=atoi(sPort+3)-1;
		sprintf(devname,"/dev/ttyS%d",res);
	}else if (strnicmp(sPort,"ttys",4)==0){
		res=atoi(sPort+4);
		sprintf(devname,"/dev/ttyS%d",res);
	}else if (strnicmp(sPort,"ttyusb",6)==0){
		res=atoi(sPort+6);
		sprintf(devname,"/dev/ttyUSB%d",res);
	}else if (strnicmp(sPort,"tty",3)==0){
		sprintf(devname,"/dev/%s",sPort);
		STP p = stlSetStf("%s",sPort);
		stlLeaveMult(p,"0123456789");
		res = atoi(p->sBuf);
		stlFree(p);
	}

	if (res<0) return 0;

	if (stlSerialDevLock(devname)) return 0;
	
	devFd = open(devname, O_RDWR|O_NDELAY);
	if (devFd >= 0){
		/* Cancel the O_NDELAY flag. */
		int n = fcntl(devFd, F_GETFL, 0);
		fcntl(devFd, F_SETFL, n & ~O_NDELAY);
	}
	if (devFd < 0){stlSerialDevUnlock(devname); return 0;}
	stlSerialDevHdlBind(res,devFd,devname);
	_stlSerialRaw    (devFd);
	_stlSerialCrtsRts(devFd,1);
	_stlSerialSetBlock(devFd,0);
	return devFd;
}

/* close com poort
 *	iHandle  handle of com poort
 * Return <0 error
 *         0 Ok
 */
int stlSerialClose(int iHandle)
{
	if (iHandle<=0) return -1;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return 0;
	stlSerialDevUnlockHdl(iHandle);
	close(iHandle);
	return 0;
}


static struct { int ibaud, speed; } btable[] = {
		{ 230400,	B230400 },
		{ 115200,	B115200 },
		{ 57600,	B57600 },
		{ 38400,	B38400 },
		{ 19200,	B19200 },
		{ 9600,		B9600 },
		{ 4800, 	B4800 },
		{ 4800, 	B4800 },
		{ 2400,		B2400 },
		{ 1200,		B1200 },
		{ 600,		B600 },
		{ 300,		B300 },
		{ 200,		B200 },
		{ 150,		B150 },
		{ 110,		B110 },
		{ 75,		B75 },
		{ 50,		B50 },
		{ 0,		B0 }
}; 
static struct { int idata, cdata; } dtable[] ={
		{ 8,	CS8 },
		{ 7,	CS7 },
		{ 6,	CS6 },
		{ 5,	CS5 },
		{ 0,	0 }
}; 



/* Set baudrate of serial port
 *	iHandle  socket (handel) to serial port
 *  buad   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
 *  parity parity bits (N,O,E) none,odd,even
 *  data   data bists (6,7,8)
 *  stop   stop bits  (1,2)
 * Return 0 ok
 *       -1 can not get serial params
 *       -2 can not set serial params
 *       -3 illegal socket number
 */
int stlSerialSetParams(int iHandle,int baud,int parity,int databits,int stopbits)
{
	struct termios tio;
	speed_t speed = B38400;
	int i;

	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return 0;
	if (iHandle<0) return -3;

	for (i = 0; btable[i].ibaud; i++)
		if (baud == btable[i].ibaud){
			speed = btable[i].speed;
			break;
		}

	for (i = 0; dtable[i].idata; i++)
		if (databits == dtable[i].idata){
			databits = dtable[i].cdata;
			break;
		}

	switch (parity){
		case 'e' :
		case 'E' : parity = PARENB; break;
		case 'o' :
		case 'O' : parity = PARENB | PARODD; break;
		default  : parity = IGNPAR; break;
	}

	if (tcgetattr (iHandle, &tio)<0) return -1;
	tio.c_cflag &= ~CSIZE;
	tio.c_cflag |= databits;
	tio.c_iflag = (parity == IGNPAR) ? (tio.c_iflag | IGNPAR) : (tio.c_iflag & ~IGNPAR);
	tio.c_cflag = (parity == IGNPAR) ? tio.c_cflag : (tio.c_cflag | parity);
	tio.c_cflag = (stopbits == 2) ? (tio.c_cflag | CSTOPB) : (tio.c_cflag & ~CSTOPB);
	if (cfsetispeed (&tio, speed)<0) return -2;
	if (cfsetospeed (&tio, speed)<0) return -2;
	if (tcsetattr (iHandle, TCSANOW, &tio)<0) return -2;
	return 0;
}

/* Read data from serial port
 *	iHandle  handle naar serial port
 *  buf    buffer to read data in
 *  imax   max numver of chars to read.
 * Return >=0 number of bytes read
 *        -1  Illegal handle
 *        -2  Read error (illegal handle ??)
 */
int stlSerialRead(int iHandle,char *buf,int imax)
{
	int res;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return _serSimRead(iHandle,buf,imax);
	if (iHandle<0) return -1;
	res=read(iHandle,buf,imax);
	if (res<0){
		if (errno == EAGAIN) return 0;	// no data in buffer
		printf("Error reading from rs232 port %d,%d\n",res,errno);
		return -2;
	}
//{FILE *f1=fopen("/dev/null","wb");if (f1) fclose(f1);}	// Is nodig voor USB rs232 converter in virtual linux box
	return res; 
}

/* Write data to serial port
 *	iHandle  handle naar serial port
 *  buf    buffer of data to write
 *  iLen   number of bytes to write
 * Return >=0  number of bytes written
 *        -1  Illegal handle
 *        -2  Write error (illegal handle ??)
 */
int stlSerialWrite(int iHandle,const char *buf,int iLen)
{
	int res;
	if ((iHandle>=_SimHdlBase_)&&(iHandle<_SimHdlBase_+_MaxSim_)) return _serSimWrite(iHandle,buf,iLen);
	if (iHandle<0) return -1;
	res=write(iHandle,buf,iLen);
	if ((res<0)&&(errno==EAGAIN)) return 0;
//{FILE *f1=fopen("/dev/null","wb");if (f1) fclose(f1);}	// Is nodig voor USB rs232 converter in virtual linux box
	if (res< 0){
		printf("Error writing to rs232 port %d,%d\n",res,errno);
		return -2;
	}
	return res; 
}

#endif

/* Open com poort
**	sPort naam van poort COM1-COM99 of ttyS0-ttyS99
**  baud   baudrate (115200,57600,38400,19200,9600,4800,2400,1200,600,300,110)
**  parity parity bits (N,O,E) none,odd,even
**  data   data bits (6,7,8)
**  stop   stop bits  (1,2)
** Return 0  error 
**        >  handle (socket naar poort)
*/
int stlSerialOpenEx(const char *sPort,int baud,int parity,int databits,int stopbits)
{
	int iHdl = stlSerialOpen(sPort);
	if (iHdl==0) return 0;
	if (stlSerialSetParams(iHdl,baud,parity,databits,stopbits)<0)
	{
		stlSerialClose(iHdl);
		return 0;
	}
	return iHdl;
}


#define _SerialMagic_	0x137AD7F3

/* Initialiseer serial buffered reader
**  iHdl  handle to serial port fd
** Return NULL error
**        stlSerial pointer
**/
stlSerial * stlSerialInit(int iHdl)
{
	stlSerial *sl;
	if (iHdl <= 0) return NULL;
	sl = (stlSerial *) malloc(sizeof(stlSerial));
	if (sl==NULL) return NULL;
	memset(sl,0,sizeof(stlSerial));
	stlMutexInit(&sl->mux);
	sl->iHdl  =iHdl;
	sl->iMagic=_SerialMagic_;
	return sl;
}


/* Lock a connection for private use in al multi threaded program.
*/
void stlSerialLock(stlSerial *sl)
{
	if ((sl)&&(sl->iMagic==_SerialMagic_))
		stlMutexLock(&sl->mux);
}
/* Unlock a connection for private use in al multi threaded program.
*/
void stlSerialUnlock(stlSerial *sl)
{
	if ((sl)&&(sl->iMagic==_SerialMagic_))
		stlMutexUnlock(&sl->mux);
}


/* Release serial buffer read data
** Also closes the serial port
**/
void stlSerialRelease(stlSerial *sl)
{
	if ((sl)&&(sl->iMagic==_SerialMagic_))
	{
		stlSerialClose(sl->iHdl);
		memset(sl,0,sizeof(stlSerial));
	}
}
/* read data from serial port
**  sl        pointer to serial buffer struct
**  iTimeout  Read timeout in ms (-1 wait forever)
** Return 0 ok data read
**       -1 timeout
**/
#ifdef _WIN32
static int _stlSerialRead(stlSerial *sl,int iTimeOutMSec)
{
	int iLen;
	int iTim=stlMsTimer(0);
	while(1)
	{
		iLen=stlSerialRead(sl->iHdl,sl->sBuf,sizeof(sl->sBuf));
		if (iLen<0) return iLen;
		if (iLen>0) break;
		// controleer alleen voor timeout als deze >=0 is
		if (iTimeOutMSec==0) return -1;		// direct return
		if ((iTimeOutMSec>0)&&(stlMsTimer(iTim)>=(unsigned int)iTimeOutMSec)) return -1;	// timeout
		stlMsWait(1);
	}
	sl->iBufLen=iLen;
	sl->iBufOfs=0;
	return 0;
}
#endif
#if defined(__linux__) || defined(__APPLE__)
static int _stlSerialRead(stlSerial *sl,int iTimeOutMSec)
{
	int iLen;
	struct timeval tv;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET((unsigned int)sl->iHdl, &fds);
	if (iTimeOutMSec>=0)
	{
		// Calculate timeout for select
		tv.tv_sec = iTimeOutMSec/1000;
		tv.tv_usec=(iTimeOutMSec%1000)*1000;
		iLen = select(sl->iHdl+1, &fds, NULL, NULL, &tv);	// met timeout
	}
	else
		iLen = select(sl->iHdl+1, &fds, NULL, NULL, NULL);	// oneindig wachten
	if (iLen<=0) return -1;			// timeout
	iLen=stlSerialRead(sl->iHdl,sl->sBuf,sizeof(sl->sBuf));
	if (iLen<0) return iLen;
	sl->iBufLen=iLen;
	sl->iBufOfs=0;
	return 0;
}	
#endif


/* Read 1 character from serial port
**  sl        pointer to serial buffer struct
**  iTimeout  Read timeout in ms (-1 wait forever)
** Return 0 ok data read
** 
**/
int stlSerialGetch(stlSerial *sl,int iTimeOutMSec)
{
	int iRes;
	if ((sl==NULL)||(sl->iMagic!=_SerialMagic_)) return -3;	// invalid stlSerial struct ptr
	if (sl->iBufOfs>=sl->iBufLen)
	{
		iRes=_stlSerialRead(sl,iTimeOutMSec);
		if (iRes<0) return iRes;
	}
	return (sl->sBuf[sl->iBufOfs++])&0xFF;
}
/* Clear alle characters in read buffer of serial line
**  sl        pointer to serial buffer struct
** Return 0 ok data read
**       -3 invalid buffer struct
**/
int stlSerialRxClear(stlSerial *sl)
{
	if ((sl==NULL)||(sl->iMagic!=_SerialMagic_)) return -3;	// invalid stlSerial struct ptr
	while (_stlSerialRead(sl,0)==0);
	sl->iBufLen=0;
	sl->iBufOfs=0;
	if (sl->sLine)
	{
		stlFree(sl->sLine);
		sl->sLine=NULL;
	}
	return 0;
}

/* Write string to serial port (like printf)
**  iHdl  handle to serial port fd
**  fmt   format string like printf
** Return >=0  number of bytes written
**        -1  Illegal handle
**        -2  Write error (illegal handle ??)
**/
int stlSerialWritef(int iHdl,char *fmt,...)
{
	int iRes;
	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	iRes=stlSerialWrite(iHdl,pVA->sBuf,pVA->iLen);
	stlFree(pVA);
	return iRes;
}
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
int stlSerialReadMax(stlSerial *sl,void *buf,int iLen,int timeout1,int timeout2,const char *sIgnore,const char *sStop,int *iReadCnt)
{
	char *sBuf=(char*)buf;
	int iOfs=0,iRes;
	if ((sl==NULL)||(sl->iMagic!=_SerialMagic_)) return -3;	// invalid stlSerial struct ptr
	if (buf==NULL) return -3;
	while (iOfs<iLen){
		if (sl->iBufOfs>=sl->iBufLen){
			if (iOfs)iRes=stlSerialGetch(sl,timeout2);	// Timeout if we got any data
			else     iRes=stlSerialGetch(sl,timeout1);	// Timeout for first char to arrive
			if (iRes<0){
				if (iReadCnt) *iReadCnt=iOfs;			// Number read bytes
				return iRes;
			}
		}else{
			iRes=sl->sBuf[sl->iBufOfs++];
		}
		// strchr geeft op een 0 character altijd waar dus deze is nu uitgesloten
		if ((iRes) && (sIgnore) && (strchr(sIgnore,iRes))) continue;	// ignore char
		if ((iRes) && (sStop  ) && (strchr(sStop  ,iRes))){
			if (iReadCnt) *iReadCnt=iOfs;
			return iRes;																// Stop char hit
		}
		sBuf[iOfs++]=iRes;
	}
	if (iReadCnt) *iReadCnt=iOfs;
	return 0;
}



/* Read string struct from serial connection stream
**	sl      communication structure
**  timeout1 time to wait for first character to arrive      (<0 wait indefinitely)
**  timeout2 time to wait for following characters to arrive (<0 wait indefinitely)
**  sIgnore  string of characters to ignore in data stream.           (optional)
**  sStop    string of chars to stop on.                              (optional)
** return: NULL timeout or read error
**         Data data was read til stop char was reached
*/
STP stlSerialReadLineEx(stlSerial *sl,int timeout1,int timeout2,const char *sIgnore,const char *sStop)
{
	int iOfs,iLen,iCnt,iExtra=256,ch;
	STP Lin;
	if ((sl==NULL)||(sl->iMagic!=_SerialMagic_)) return NULL;	// invalid stlSerial struct ptr

	if (sl->sLine){						// We already have some data (continue using that)
		Lin=sl->sLine;					// Use that data
		sl->sLine=NULL;
		iOfs=Lin->iLen;
		iLen=iOfs+iExtra;				// Add extra space to buffer
		stlInsDel(Lin,iOfs,iLen);
	}else{
		iOfs=0;							// We start with new data
		iLen=iOfs+iExtra;				// Some extra space 
		Lin=_strTlInitVar(iLen);		// Initialize
	}

	while (1){
		_strTlLockVar(Lin);
		ch=stlSerialReadMax(sl,&Lin->sBuf[iOfs],iExtra,timeout1,timeout2,sIgnore,sStop,&iCnt);
		iOfs+=iCnt;
		Lin->iLen=iOfs;					// Set actual count in struct
		Lin->sBuf[iOfs]=0;				// Add terminating 0
		_strTlUnlockVar(Lin);			// Unlock
		if (ch<0) break;				// Timeout or error
		if (ch) return Lin;				// Stop character reached
		iExtra *= 2;					// Expand some more
		iLen=iOfs+iExtra;
		_strTlInsDel(Lin,iOfs,iLen);
		timeout1=timeout2;				// We have some data so timeout1 needs to be same as timeout2
	}
	sl->sLine=Lin;
	return NULL;
}

/* Read string struct from serial connection stream
**	sl       communication structure
**  timeout  time to wait for characters to arrive (resets after every char)(<0 wait indefinitely)
** return: NULL timeout or read error
**         Data line of chars was rad from serial port stream
*/
STP stlSerialReadLine(stlSerial *sl,int timeout)
{
	return stlSerialReadLineEx(sl,timeout,timeout,"\r","\n");
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

rs232::rs232(stlSerial *ser)
{
	initVars();
	sl = ser;
	bSerCleanup = false;
}
rs232::rs232(const char *portName,int baud /* = 9600 */,int parity /* = 'n'  */,int databits /* = 8 */,int stopbits /* = 1 */)
{
	initVars();
	iHdl = stlSerialOpenEx(portName,baud,parity,databits,stopbits);
	if (iHdl > 0) sl = stlSerialInit(iHdl);
}
rs232::rs232(int portNr,int baud /* = 9600 */,int parity /* = 'n'  */,int databits /* = 8 */,int stopbits /* = 1 */)
{
	initVars();
	cST pn;
	pn.setf("COM%d",portNr);
	iHdl = stlSerialOpenEx(pn,baud,parity,databits,stopbits);
	if (iHdl > 0) sl = stlSerialInit(iHdl);
}
rs232::~rs232()
{
	if ((sl) && (bSerCleanup)) stlSerialRelease(sl);
	initVars();
}

int rs232::write(const char *buf         ,int bufLen /* = -1 */)
{
	if ((bufLen < 0) && (buf != NULL)) bufLen = strlen(buf);
	return stlSerialWrite(iHdl,buf,bufLen);
}
int rs232::write(const unsigned char *buf,int bufLen /* = -1 */)
{
	if ((bufLen < 0) && (buf != NULL)) bufLen = strlen((const char*)buf);
	return stlSerialWrite(iHdl,(const char*)buf,bufLen);
}
int rs232::write(STP buf)
{
	return stlSerialWrite(iHdl,buf->sBuf,buf->iLen);
}
int rs232::write(cST &buf)
{
	if (buf.is8bit())
		return write(buf.buf(),buf.length());
	return write((const char *) buf.bufW(),buf.length() * 2);
}
int rs232::write(cST *buf)
{
	if (buf == NULL) return -1;
	if (buf->is8bit())
		return write(buf->buf(),buf->length());
	return write((const char *) buf->bufW(),buf->length() * 2);
}
int rs232::writef(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP pVA=stlSetSta(fmt,ap);
	va_end(ap);
	int iRes=stlSerialWrite(iHdl,pVA->sBuf,pVA->iLen);
	stlFree(pVA);
	return iRes;
}
int rs232::getc(int timeout /* = -1 */)
{
	return stlSerialGetch(sl,timeout);
}
STP rs232::readLine(int timeout1 /* = -1 */,int timeout2 /* = -1 */,const char *sIgnore /* = "\r"  */,const char *sStop /* = "\n" */)
{
	return stlSerialReadLineEx(sl,timeout1,timeout2,sIgnore,sStop);
}
int rs232::read(void *buf,int iLen,int timeout1 /* = -1 */,int timeout2 /* = -1 */,const char *sIgnore /* = NULL */,const char *sStop /* = NULL */,int *iReadCnt /* = NULL */)
{
	return stlSerialReadMax(sl,buf,iLen,timeout1,timeout2,sIgnore,sStop,iReadCnt);
}
STP rs232::read(int timeout1 /* = -1 */,int timeout2 /* = -1 */)
{
	STP sRes = stlSerialReadLineEx(sl,timeout1,timeout2,NULL,NULL);
	if ((sRes == NULL) && (sl) && (sl->sLine)){ sRes = sl->sLine; sl->sLine = NULL; }		// in geval van timeout gaan we gelezen data terug geven
	return sRes;
}
STP rs232::readLen(int numBytes,int timeout1 /* = -1 */,int timeout2 /* = -1 */)
{
	if (numBytes > 0)
	{
		STP res = stlInitLen(numBytes,0);
		int iCnt = 0;
		int iRes = stlSerialReadMax(sl,res->sBuf,res->iLen,timeout1,timeout2,NULL,NULL,&iCnt);
		if (iRes == 0) return res;
		if (iRes > 0){res->iLen = iRes; return res;}
		stlFree(res);
		return NULL;
	}
	return NULL;
}
