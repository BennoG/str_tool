

#ifdef _WIN32
#  include <conio.h>
#endif

#ifdef __linux__
// set optional input device e.g. /dev/input/event0		
// return -1 unable to open
//        -2 already open
//         0 ok opened
int _kbDevInput(const char *devName);
int _kbhit(void);
int _getch(void);
#endif	//  __linux__


