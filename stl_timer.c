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
#if defined (_WIN32)
#  include <windows.h>
#  include <intrin.h>
#pragma intrinsic(__rdtsc)
#endif

#if defined (__linux__)
#  include <sys/time.h>
#  include <unistd.h>
#endif

#include <stdio.h>
#include <time.h>

#include "stl_thread.h"
#include "internal.h"

#if defined (__linux__)
unsigned int stlUsTimer(unsigned int uOld)
{
    unsigned int uNew;
    struct timeval tv;
    gettimeofday(&tv,NULL);
	uNew =tv.tv_sec * 1000000;
	uNew+=tv.tv_usec;
	return (uNew-uOld);
}
unsigned int stlMsTimer(unsigned int uOld)
{
    unsigned int uNew;
    struct timeval tv;
    gettimeofday(&tv,NULL);
	uNew =tv.tv_sec * 1000;
	uNew+=tv.tv_usec/ 1000;
	return (uNew-uOld);
}
#ifdef __arm__
#else
#  ifdef __aarch64__
#  else
unsigned __int64 __rdtsc()
{
	unsigned __int64 x;
	__asm__("rdtsc\n\t"
		"mov %%edx, %%ecx\n\t"
		:"=A" (x));
	return x;
}
#  endif //__aarch64__
#endif //__arm__

#endif


long long stlMsTimerEpoch()
{
	/* weet niet of dit wel sneller is werkt alleen onder C++(11)
	using namespace std::chrono;
	milliseconds ms = duration_cast< milliseconds >(
	system_clock::now().time_since_epoch()
	);
	*/
#ifdef __linux__
	struct timeval tp;
	long long mslong;
	gettimeofday(&tp, NULL);
	mslong = (long long) tp.tv_sec * 1000LL + tp.tv_usec / 1000; //get current timestamp in milliseconds
	return mslong;
#else
	long long ms;
	FILETIME ft;
	LARGE_INTEGER li;
	GetSystemTimeAsFileTime(&ft);			// voor rekenaars dit loopt pas over in 30828 (geen typo) we hebben dus nog een kleine 28000 jaar te gaan.
	li.HighPart = ft.dwHighDateTime;
	li.LowPart  = ft.dwLowDateTime;
	ms = li.QuadPart / 10000;				// QuadPart is in 100ns ticks
	ms -= 11644473600000LL;					// verschil Linux epoch en windows epoch in ms
	return ms;
#endif
}



#if defined (_WIN32)
unsigned int stlUsTimer(unsigned int uOld)
{
	unsigned int uNew;
	static double freq=0;
	LARGE_INTEGER li;
	if (freq==0){
		if (QueryPerformanceFrequency(&li)){
			freq=li.QuadPart/1000000.0;
		}else{
			printf("No highspeed timer\n");
			_strSafeExit(_StTlNoHighSpeedTimer_);
		}
	}
	QueryPerformanceCounter(&li);
	uNew=(unsigned int)(li.QuadPart/freq);
	return (uNew-uOld);
}
unsigned int stlMsTimer(unsigned int uOld)
{
	unsigned int uNew;
	static double freq=0;
	LARGE_INTEGER li;
	if (freq==0){
		if (QueryPerformanceFrequency(&li)){
			freq=li.QuadPart/1000.0;
        }else{
            printf("No highspeed timer\n");
			_strSafeExit(_StTlNoHighSpeedTimer_);
        }
	}
	QueryPerformanceCounter(&li);
	uNew=(unsigned int)(li.QuadPart/freq);
	return (uNew-uOld);
}
#endif

#ifdef __arm__
unsigned int stlCpuPerMs(int recalc)
{
	return 1000000;
}
#else
#  ifdef __aarch64__
unsigned int stlCpuPerMs(int recalc)
{
	return 1000000;
}
#  else
unsigned __int64 stlCpuTimer64(unsigned __int64 uOld)
{
	unsigned __int64 uNew = __rdtsc();
	return (uNew - uOld);
}
unsigned int stlCpuTimer(unsigned int uOld)
{
	unsigned __int64 uNew64 = __rdtsc();
	unsigned int uNew = (unsigned int)(uNew64 >> 16);
	return (uNew - uOld);
}

static unsigned int stlCpuPerMsSave = 0;
unsigned int stlCpuPerMs(int recalc)
{
	if ((stlCpuPerMsSave == 0) || (recalc))
	{
		unsigned int c1,c2;
		unsigned __int64 timCpu = stlCpuTimer64(0);
		unsigned int timUs = stlUsTimer(0);
		stlMsWait(100);
		timCpu = stlCpuTimer64(timCpu);
		timUs  = stlUsTimer(timUs);
		c1 =(unsigned int)(1000 * timCpu / timUs);
		timCpu = stlCpuTimer64(0);
		timUs = stlUsTimer(0);
		stlMsWait(100);
		timCpu = stlCpuTimer64(timCpu);
		timUs  = stlUsTimer(timUs);
		c2 =(unsigned int)(1000 * timCpu / timUs);
		stlCpuPerMsSave = (c1 + c2) / 2;
		// printf("stlCpuPerMs %d %d",c1,c2);
	}
	return stlCpuPerMsSave;
}
#  endif // __aarch64__
#endif	// __arm__
