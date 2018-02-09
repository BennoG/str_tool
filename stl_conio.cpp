#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stl_conio.h"
#include "stl_thread.h"

#ifdef __linux__

#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>      /* open()   O_RDONLY ... */
#include <string.h>     /* strerror() */
#include <errno.h>      /* errno */
#include <linux/input.h>    /* EVIOCGVERSION ++ */

class linuxConio
{
private:
	void initVars(){chGet = 0; threadState = 0; memset(&oldTIO,0,sizeof(oldTIO)); haveOldTIO = false; escMs = 0; inputFD = -1;}
private:
	bool haveOldTIO;
	struct termios oldTIO;
	int escMs;				// ESC teken ontvangen in canMsTimer;
	int threadState;
	int chGet;
	// voor device
	int inputFD;
public:
	linuxConio();
	~linuxConio();
	int _kbhit(void);
	int _getch(void);
	int setInputDevice(const char *devName);
private:
	static void workThread(void *pUserData);
	static void workThreadFD(void *pUserData);
	static void handleCtrlC(int sig);
	int lxRawKeys(int iRaw,int iEcho);
	int lxRestoreKeys();
};

static linuxConio cio;

linuxConio::linuxConio()
{
	initVars();
	// Register signals 
	signal(SIGINT , handleCtrlC);
	signal(SIGTERM, handleCtrlC);
	threadState = 1;
	stlThreadStart(workThread,20,this,"linuxConio");
}
linuxConio::~linuxConio()
{
	if (inputFD >= 0)
	{
		int fd = inputFD;
		inputFD = -1;
		close(fd);
	}
	for (int i = 0; i < 250; i++){		// wacht max 2.5 sec tot taak is opgestart
		if (threadState >= 2) break;
		stlMsWait(1);
	}
	threadState = 3;					// geef aan dat taak moet stoppen.
	for (int i = 0; i < 250; i++){		// wacht max 2.5 sec tot taak is gestopt
		if (threadState > 3) break;
		stlMsWait(1);
	}
	printf("restoring console settings");
	lxRestoreKeys();
}

void linuxConio::workThread(void *pUserData)
{
	linuxConio *c = static_cast<linuxConio*>(pUserData);
	if (c)
	{
		c->threadState = 2;
		c->lxRawKeys(1,0);				// zet toetsenbord in rauwe mode (direct op ESC regeren etc)
		int escLen = 0;	// aantal tekens voor ESC
		int escPos = 0;	// huidige teken
		int escVal = 0;	// huidige waarde
		while (c->threadState == 2)
		{
			int ch = getchar();
			if (ch <   0) { stlMsWait(500); continue; }

			//printf("ch(0x%X)\n",ch);
			// F1 = 1B,5B,31,31,7E
			// F2 = 1B,5B,31,32,7E
			// F10= 1B,5B,32,31,7E
			// F12= 1B,5B,32,31,7E

			if ((ch == 27) && (c->escMs == 0))
			{
				escPos = escVal = 0;
				escLen = 2;
				c->escMs = stlMsTimer(0);
				continue;
			}
			if (c->escMs) {
				if ((escPos == 1) && (ch == 0x31)) escLen = 4;
				if ((escPos == 1) && (ch == 0x32)) escLen = 4;
				escVal = (escVal << 8) | ch;
				escPos++;
				if (escPos < escLen){ c->escMs = stlMsTimer(0); continue; }
				c->escMs = 0;
				ch = escVal & 0xFFFFFF;
				//printf("ch done (0x%X)",escVal);
			}
			c->chGet = ch;
		}
		c->threadState = 4;
	}
}
void linuxConio::handleCtrlC(int sig)
{
	cio.escMs = 0;
	cio.chGet = 27;
}

int linuxConio::lxRawKeys(int iRaw,int iEcho)
{
	struct termios old;
	memset(&old,0,sizeof(old));
	if (tcgetattr(0, &old) < 0) return -1;
	if (!haveOldTIO)
	{
		haveOldTIO = true;
		oldTIO = old;
	}
	if (iRaw){
		old.c_iflag &= ~IXOFF;    // found somewhere else 
		old.c_lflag &= ~ICANON;
	}else
		old.c_lflag |= ICANON;
	if (iEcho)
		old.c_lflag |= ECHO;
	else
		old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &old) < 0) return -2;
	return 0;
}

int linuxConio::lxRestoreKeys()
{
	if (haveOldTIO)
	{
		if (tcsetattr(0, TCSANOW, &oldTIO) < 0) return -2;
	}
	return 0;
}

int linuxConio::_kbhit(void)
{
	if ((escMs) && (stlMsTimer(escMs) > 100)) return 27;
	return chGet;
}
int linuxConio::_getch(void)
{
	if ((escMs) && (stlMsTimer(escMs) > 100)){escMs = 0; return 27;}
	while (chGet == 0) stlMsWait(1);
	int iRes = chGet;
	chGet = 0;
	return iRes;
}

#define EV_BUF_SIZE 16

/*
numpad 0 down
1471629625.377812: type=04 code=04 value=70053
1471629625.377812: type=01 code=45 value=01				KEY_NUMLOCK
1471629625.377812: type=04 code=04 value=70062
1471629625.377812: type=01 code=52 value=01				KEY_KP0
1471629625.377812: type=00 code=00 value=00

1471629625.385783: type=11 code=00 value=01
1471629625.385783: type=04 code=04 value=70053
1471629625.385783: type=01 code=45 value=00
1471629625.385783: type=00 code=00 value=00

numpad 0 up
1471629626.777983: type=04 code=04 value=70062
1471629626.777983: type=01 code=52 value=00
1471629626.777983: type=04 code=04 value=70053
1471629626.777983: type=01 code=45 value=01
1471629626.777983: type=00 code=00 value=00

1471629626.785965: type=11 code=00 value=00
1471629626.785965: type=04 code=04 value=70053
1471629626.785965: type=01 code=45 value=00
1471629626.785965: type=00 code=00 value=00

Enter down
1471629759.715400: type=04 code=04 value=70028    EV_MSC
1471629759.715400: type=01 code=1c value=01       EV_KEY     1C = 28 == KEY_ENTER    01 = down
1471629759.715400: type=00 code=00 value=00       EV_SYN

Enter up
1471629759.795378: type=04 code=04 value=70028
1471629759.795378: type=01 code=1c value=00       EV_KEY     1C = 28 == KEY_ENTER    00 = up
1471629759.795378: type=00 code=00 value=00

0F   KEY_TAB         9
62   KEY_KPSLASH     47
37   KEY_KPASTERISK  42
0E   KEY_BACKSPACE   127
4A   KEY_KPMINUS                  45
4E	 KEY_KPPLUS				 43
*/



void linuxConio::workThreadFD(void *pUserData)
{
	linuxConio *c = static_cast<linuxConio*>(pUserData);
	if (c)
	{
		struct input_event ev[EV_BUF_SIZE]; /* Read up to N events ata time */

		while (c->threadState == 2)
		{
			int sz = read(c->inputFD, ev, sizeof(struct input_event) * EV_BUF_SIZE);

			if (sz < (int) sizeof(struct input_event)) {
				printf("ERR %d:\n%s\n",	errno, strerror(errno));
				break;
			}

			/* Implement code to translate type, code and value */
			for (unsigned int i = 0; i < sz / sizeof(struct input_event); ++i) {

				if ((ev[i].type == EV_KEY) && (ev[i].value == 1))	// value 2 = auto repeat
				{
					switch (ev[i].code)
					{
					case KEY_ENTER:		cio.escMs = 0; cio.chGet =  10; break;
					case KEY_KP0:		cio.escMs = 0; cio.chGet = '0'; break;
					case KEY_KP1:		cio.escMs = 0; cio.chGet = '1'; break;
					case KEY_KP2:		cio.escMs = 0; cio.chGet = '2'; break;
					case KEY_KP3:		cio.escMs = 0; cio.chGet = '3'; break;
					case KEY_KP4:		cio.escMs = 0; cio.chGet = '4'; break;
					case KEY_KP5:		cio.escMs = 0; cio.chGet = '5'; break;
					case KEY_KP6:		cio.escMs = 0; cio.chGet = '6'; break;
					case KEY_KP7:		cio.escMs = 0; cio.chGet = '7'; break;
					case KEY_KP8:		cio.escMs = 0; cio.chGet = '8'; break;
					case KEY_KP9:		cio.escMs = 0; cio.chGet = '9'; break;
					case KEY_TAB:		cio.escMs = 0; cio.chGet =   9; break;
					case KEY_KPSLASH:	cio.escMs = 0; cio.chGet =  47; break;
					case KEY_KPASTERISK:cio.escMs = 0; cio.chGet =  42; break;
					case KEY_BACKSPACE:	cio.escMs = 0; cio.chGet = 127; break;
					case KEY_KPMINUS:	cio.escMs = 0; cio.chGet =  45; break;
					case KEY_KPPLUS:	cio.escMs = 0; cio.chGet =  43; break;
					case KEY_KPDOT:		cio.escMs = 0; cio.chGet = '.'; break;
					case KEY_NUMLOCK:   break;
					default:
						printf("linuxConio::workThreadFD type=%02x code=%02x value=%02x\n",ev[i].type,ev[i].code,ev[i].value);
						break;
					}
				}
			}
		}
		if (c->inputFD >= 0) close(c->inputFD);
		c->inputFD = -1;
	}
}


int linuxConio::setInputDevice(const char *devName)
{
	if ((inputFD >= 0) && (devName == NULL))
	{
		int fd = inputFD;
		inputFD = -1;
		close(fd);
		stlMsWait(50);
		return 0;
	}
	if (inputFD >= 0) return -2;
	if ((inputFD = open(devName, O_RDONLY)) < 0) {
		printf(	"ERR %d:\n"
				"Unable to open `%s'\n"
				"%s\n",
				errno, devName, strerror(errno) );
		return -1;
	}
	/* Error check here as well. */
	/*
	ioctl(inputFD, EVIOCGVERSION, &version);
	ioctl(inputFD, EVIOCGID, id); 
	ioctl(inputFD, EVIOCGNAME(sizeof(name)), name);

	fprintf(stderr,
		"Name      : %s\n"
		"Version   : %d.%d.%d\n"
		"ID        : Bus=%04x Vendor=%04x Product=%04x Version=%04x\n"
		"----------\n"
		,
		name,

		version >> 16,
		(version >> 8) & 0xff,
		version & 0xff,

		id[ID_BUS],
		id[ID_VENDOR],
		id[ID_PRODUCT],
		id[ID_VERSION]
	);
	*/
	stlThreadStart(workThreadFD,20,this,"linuxConioFD");
	return 0;
}



// http://unix.stackexchange.com/questions/94322/is-it-possible-for-a-daemon-i-e-background-process-to-look-for-key-presses-fr
int _kbDevInput(const char *devName)
{
	//if (cio == NULL) cio = new linuxConio();
	return cio.setInputDevice(devName);
}

int _kbhit(void)
{
	//if (cio == NULL) cio = new linuxConio();
	return cio._kbhit();
}
int _getch(void)
{
	//if (cio == NULL) cio = new linuxConio();
	return cio._getch();
}
#endif
