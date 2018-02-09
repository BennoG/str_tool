#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int lxRawKeys(int iRaw,int iEcho)
{
	struct termios old = {0};
	if (tcgetattr(0, &old) < 0) return -1;
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


char _getch() 
{
        char buf = 0;
        if (read(0, &buf, 1) < 0) return -1;
        return (buf);
}



int main()
{
    lxRawKeys(1,0);
    while (1)
    {
        //int ch = _getch();
        int ch = getchar();
        printf("ch = %d\n",ch);
        if (ch == 'a') break;
    }
    lxRawKeys(0,1);
    return 0;
}

