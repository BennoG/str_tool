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

#include <time.h>
#ifdef __linux__
#  include <sys/time.h>
#  include <unistd.h>
#endif
#ifdef _WIN32
#  include <WinError.h>
#endif
#include <stdlib.h>

#include "stl_str.h"

/* Convert date to integer
 *  yy  year (4 digits if < 100 then 2000 is added to year)
 *  mm  month
 *  dd  day of mont
 * Return number of day since 1 Jan 1968
 */
int stlDateToInt(int yy,int mm,int dd)
{
	int dag,i;
	int MonthDays[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	if (yy<100) yy+=2000;
	
	// Ga er hieronder vanuit dan yy altijd in 4 cijferig formaat is.
	if ((yy % 4)==0)  MonthDays[1]=29; else MonthDays[1]=28;
	if (mm< 1) mm= 1;
	if (mm>12) mm=12;
	if (dd< 1) dd= 1;
	if (dd>MonthDays[mm-1]) dd=MonthDays[mm-1];
	
	yy -=1968;
	dag =(yy%4)*365;
	dag+=(yy/4)*1461;
	if ((yy>=0) && (yy%4)) dag++;
	
	for (i=1; i<mm; i++) dag+=MonthDays[i-1];
	
	dag+=dd;
	
	return dag;
}

// 1 = 01-01-1968 = julian 2439857
//     13-11-2006 = julian 2454053
/* Convert integer date back to actual date base date = 1 Jan 1968
 *  day Integer representation of date
 *  pyy (opt) integer pointer to get year (4 digits)
 *  pmm (opt) integer pointer to get month
 *  pdd (opt) integer pointer to get day of month
 */
void stlIntToDate(int day,int *pyy,int *pmm,int *pdd)       // 1 = 01-01-1968 
{
	int MonthDays[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	int yy,mm;
	yy   = (day-1) / 1461;
	day -= yy * 1461;
	yy   = (yy * 4) + 1968;
	
	while (1){
		if (day > 366) {yy++; day-=366;}  else break;
		if (day > 365) {yy++; day-=365;}  else break;
		if (day > 365) {yy++; day-=365;}
		break;
	}
	while (1){
		if (day <=0) {yy--; day+=365;}  else break;
		if (day <=0) {yy--; day+=365;}  else break;
		if (day <=0) {yy--; day+=365;}  else break;
		if (day <=0) {yy--; day+=366;}
		break;
	}
	
	if ((yy % 4)==0)  MonthDays[1]=29; else MonthDays[1]=28;
	mm=0;
	while (1){
		if (day <= MonthDays[mm]) { mm++; break; }
		day -= MonthDays[mm];
		mm++;
	}
	
	if (pyy) *pyy=yy;
	if (pmm) *pmm=mm;
	if (pdd) *pdd=day;
}

/* Calculate weekday from integer date.
 *  iDate date from stlDateToInt
 * Return (1=Sun 2=Mon 3=Tue 4=Wed 5=Thu 6=Fri 7=Sat)
 */
int stlIntTowWeekday(int iDate)
{
	while (iDate<0) iDate+=7000;
	return ((iDate%7)+1);
}

/* Calculate date from week number (customizable)
**  iWeek     Week number YYYYWW or WW
**  iFirstDay First day of the week (1-7) (2=Monday default)(0)
**  iMinDays  Minimum number of days in first week of a year (4=default)(0)
** Return Date in integer format
*/
int stlWeekToDateEx(int iWeek,int iFirstDay,int iMinDays)
{
	int h,yy;
	int iFirstofJanuary,iFirstWeekStart;

	if (iFirstDay<=0) iFirstDay=2;
	if (iMinDays<=0)  iMinDays =4;

	// Bepaal om welk jaar het gaat 
	yy=iWeek/100;
	if (yy<=0) stlDateNow(NULL,&yy,NULL,NULL);			// default huidige jaar

	// bereken eerste weekdag van jaar
	iFirstofJanuary=stlDateToInt(yy, 1, 1);
	h = stlIntTowWeekday(iFirstofJanuary-iFirstDay);	// Eerste dag van het jaar
	iFirstWeekStart=iFirstofJanuary-h;
	if (h>=iMinDays) iFirstWeekStart+=7;				// 1 Jan valt in week 52 of 53
	return iFirstWeekStart + ((iWeek-1)*7);
}

/* Calculate year week number from specific date (customizable)
 *  yy        Year (in 4 digits)
 *  mm        Month
 *  dd        Date
 *  iFirstDay First day of the week (1-7) (2=Monday default)
 *  iMinDays  Minimum number of days in first week of a year (4=default)
 * Return
 *  yyyyww
 */
int stlDateToWeekEx(int yy,int mm,int dd,int iFirstDay,int iMinDays)
{
	int h;
	int iFirstofJanuary,iFirstWeekStart,iLastWeekStart,iDate;

	if (iFirstDay<=0) iFirstDay=2;
	if (iMinDays<=0)  iMinDays =4;

	iDate=stlDateToInt(yy,mm,dd);
	
	// bereken eerste weekdag van jaar
	iFirstofJanuary=stlDateToInt(yy, 1, 1);
	h = stlIntTowWeekday(iFirstofJanuary-iFirstDay);// Eerste dag van het jaar
	iFirstWeekStart=iFirstofJanuary-h;
	if (h>=iMinDays) iFirstWeekStart+=7;						// 1 Jan valt in week 52 of 53
	
	if (iDate < iFirstWeekStart)
		return stlDateToWeekEx(yy-1,12,31,iFirstDay,iMinDays);	// Laatste dag van vorige jaar

	// bereken of deze in volgende jaar valt
	iFirstofJanuary=stlDateToInt(yy+1,1,1);
	h = stlIntTowWeekday(iFirstofJanuary-iFirstDay);// Eerste dag van volgende jaar
	iLastWeekStart=iFirstofJanuary-h;
	if (h>=iMinDays) iLastWeekStart+=7;
	if (iDate>=iLastWeekStart)
		return stlDateToWeekEx(yy+1,1,1,iFirstDay,iMinDays);// Eerste dag volgnede jaar

	return ((yy*100)+((iDate - iFirstWeekStart)/7) + 1);
}

/* Calculate year week number from specific date
 *  yy        Year (in 4 digits)
 *  mm        Month
 *  dd        Date
 * Return
 *  yyyyww
 */
int stlDateToWeek(int yy,int mm,int dd)
{
	// Monday first day of week and first week of year has minimum of 4 days
	return stlDateToWeekEx(yy,mm,dd,0,0);
}
#ifdef _WIN32
/* Sets time of operating system (local time zone)
**  iYear   (opt) year to set         <=0 don't touch
**  iMont   (opt) month to set        <=0 don't touch
**  iDay    (opt) Day of month to set <=0 don't touch
**  iHour   (opt) time hour to set    <0  don't touch
**  iMinute (opt) time minute to set  <0  don't touch
**  iMinute (opt) time second to set  <0  don't touch
** Return 0 Ok
**       -1 Error
*/
int stlDateTimeSet(int iYear,int iMonth,int iDay,int iHour,int iMinute,int iSecond)
{
	//DWORD dwErr;
	SYSTEMTIME ST;
	memset(&ST,0,sizeof(ST));
	GetLocalTime(&ST);
	if (iYear  > 0) ST.wYear  =iYear;
	if (iMonth > 0) ST.wMonth =iMonth;
	if (iDay   > 0) ST.wDay   =iDay;
	if (iHour  >=0) ST.wHour  =iHour;
	if (iMinute>=0) ST.wMinute=iMinute;
	if (iSecond>=0) ST.wSecond=iSecond;
	SetLocalTime(&ST);
	if (SetLocalTime(&ST)) return 0;	// moet twee keer om zomer / wintertijd goed te laten werken (zie MSDN doc)
	/* not working if program is not running als administrator.
	when running as administrator code below not necessary
	dwErr=GetLastError();
	if (dwErr==ERROR_PRIVILEGE_NOT_HELD)
	{
		TOKEN_PRIVILEGES tkp;
		HANDLE tkn=NULL;
		memset(&tkp,0,sizeof(tkp));
		if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&tkn))
			return -2;
		if (!LookupPrivilegeValue(NULL,SE_SYSTEMTIME_NAME,&tkp.Privileges[0].Luid))
		{
			CloseHandle(tkn);
			return -3;
		}
		tkp.PrivilegeCount=1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		SetLastError(0);
		AdjustTokenPrivileges(tkn,FALSE,&tkp,sizeof(tkp),NULL,0);
		dwErr=GetLastError();
		if (dwErr){
			CloseHandle(tkn);
			return -4;
		}
		SetLocalTime(&ST);
		if (!SetLocalTime(&ST))  	// moet twee keer om zomer / wintertijd goed te laten werken (zie MSDN doc)
		{
			dwErr=GetLastError();
			CloseHandle(tkn);
			return -1;
		}
		CloseHandle(tkn);
		return 0;
	}
	*/
	return -1;
}
#endif

#ifdef __linux__
int stlDateTimeSet(int iYear,int iMonth,int iDay,int iHour,int iMinute,int iSecond)
{
	struct timeval tv;
	struct tm *tm=NULL;
	time_t ti;
	ti=time(NULL);		// _time64       On Windows macro
	tm=localtime(&ti);	// _localtime64  On Windows macro
	if (tm){
		if (iYear>1900) tm->tm_year=iYear-1900;
		if (iMonth > 0) tm->tm_mon =iMonth-1;
		if (iDay   > 0) tm->tm_mday=iDay;
		if (iHour  >=0) tm->tm_hour=iHour;
		if (iMinute>=0) tm->tm_min =iMinute;
		if (iSecond>=0) tm->tm_sec =iSecond;
		memset(&tv,0,sizeof(tv));
		tv.tv_sec  =mktime(tm);
		if (tv.tv_sec > 0){
			settimeofday(&tv,NULL); // local time to os setting
			//set_hardware_clock_rtc_ioctl(&tm);	// set hw clock also
			if (system("hwclock --systohc")) printf("hwclock error");
			unlink("/etc/adjtime");
			return 0;
		}
	}
	return -1;
}
#endif


/* Get local or UTC time of operating system (local time zone)
**  iUtcFlg       0=localtime 1=UTC time
**  pDayNr  (opt) number of days sinds 1-1-1970
**  pYear   (opt) year
**  pMont   (opt) month
**  pDay    (opt) Day of month
**  pHour   (opt) time hour
**  pMinute (opt) time minute
**  pSecond (opt) time second
**  pMsec   (opt) number of millisecond in the sec. 
*/
void stlDateTimeGetUTC(int iUtcFlg,int *pDayNr,int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond,int *pMsec,int *tzOffset)
{
#ifdef _WIN32
	SYSTEMTIME TM;
	memset(&TM,0,sizeof(TM));
	if (iUtcFlg)
		GetSystemTime(&TM);
	else
		GetLocalTime(&TM);			// localtime houd geen rekening met zomer / wintertijd wisselingen tijdens programma run
	if (pDayNr ) *pDayNr  = stlDateToInt(TM.wYear,TM.wMonth,TM.wDay);
	if (pYear  ) *pYear   = TM.wYear;
	if (pMonth ) *pMonth  = TM.wMonth;
	if (pDay   ) *pDay    = TM.wDay;
	if (pHour  ) *pHour   = TM.wHour;
	if (pMinute) *pMinute = TM.wMinute;
	if (pSecond) *pSecond = TM.wSecond;
	if (pMsec  ) *pMsec   = TM.wMilliseconds;
#endif
#ifdef __linux__
	struct tm *tm=NULL;
	struct timeval tv;
	time_t ti;
	ti=time(NULL);		// _time64       On Windows macro
	if (pMsec) gettimeofday(&tv,NULL);
	if (iUtcFlg)
		tm=gmtime(&ti);
	else
		tm=localtime(&ti);	// _localtime64  On Windows macro
	if (tm){
		if (pDayNr ) *pDayNr = stlDateToInt(tm->tm_year+1900,tm->tm_mon +1,tm->tm_mday);
		if (pYear  ) *pYear   = tm->tm_year+1900;
		if (pMonth ) *pMonth  = tm->tm_mon +1;
		if (pDay   ) *pDay    = tm->tm_mday;
		if (pHour  ) *pHour   = tm->tm_hour;
		if (pMinute) *pMinute = tm->tm_min;
		if (pSecond) *pSecond = tm->tm_sec;
		if (pMsec  ) *pMsec   = tv.tv_usec/ 1000;
	}
#endif
	if (tzOffset)
	{
		double dj1,dj2;
		int dYY,dMM,dDD,tHH,tMM,tSS,tMS,tOfs;
		stlMsWait(0);	// thread switch if we are overdue
		stlDateTimeGetUTC(1,NULL,&dYY,&dMM,&dDD,&tHH,&tMM,&tSS,&tMS,NULL);
		dj1=stlJulianDate(dYY,dMM,dDD,tHH,tMM,tSS);
		stlDateTimeGetUTC(0,NULL,&dYY,&dMM,&dDD,&tHH,&tMM,&tSS,&tMS,NULL);
		dj2=stlJulianDate(dYY,dMM,dDD,tHH,tMM,tSS);
		if (dj1 < dj2){	// afronden op 10 sec eenheden om seconde overstap sprong te compenseren
			tOfs = ((int)((dj2-dj1)*3600*24 * 0.1 + 0.5)) * -10;
		}else{
			tOfs = ((int)((dj1-dj2)*3600*24 * 0.1 + 0.5)) * 10;
		}
		*tzOffset = tOfs;
	}
}


/*
#ifdef _WIN32
static void stlDateTimeGetEx(int *pDayNr,int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond)
{
	SYSTEMTIME TM;
	memset(&TM,0,sizeof(TM));
	GetLocalTime(&TM);			// localtime houd geen rekening met zomer / wintertijd wisselingen tijdens programma run
	if (pDayNr ) *pDayNr  = stlDateToInt(TM.wYear,TM.wMonth,TM.wDay);
	if (pYear  ) *pYear   = TM.wYear;
	if (pMonth ) *pMonth  = TM.wMonth;
	if (pDay   ) *pDay    = TM.wDay;
	if (pHour  ) *pHour   = TM.wHour;
	if (pMinute) *pMinute = TM.wMinute;
	if (pSecond) *pSecond = TM.wSecond;
}
#endif
#ifdef __linux__
static void stlDateTimeGetEx(int *pDayNr,int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond)
{
	struct tm *tm=NULL;
	time_t ti;
	ti=time(NULL);		// _time64       On Windows macro
	tm=localtime(&ti);	// _localtime64  On Windows macro
	if (tm){
		if (pDayNr ) *pDayNr = stlDateToInt(tm->tm_year+1900,tm->tm_mon +1,tm->tm_mday);
		if (pYear  ) *pYear   = tm->tm_year+1900;
		if (pMonth ) *pMonth  = tm->tm_mon +1;
		if (pDay   ) *pDay    = tm->tm_mday;
		if (pHour  ) *pHour   = tm->tm_hour;
		if (pMinute) *pMinute = tm->tm_min;
		if (pSecond) *pSecond = tm->tm_sec;
	}
}
#endif
*/

/* Get time of operating system (local time zone)
**  pYear   (opt) year
**  pMont   (opt) month
**  pDay    (opt) Day of month
**  pHour   (opt) time hour
**  pMinute (opt) time minute
**  pMinute (opt) time second
*/
void stlDateTimeGet(int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond)
{
	stlDateTimeGetUTC(0,NULL,pYear,pMonth,pDay,pHour,pMinute,pSecond,NULL,NULL);
}


/* get current date in various formats
**  pNr (opt) Integer representation of date (used in stlIntToDate and stlDateToInt)
**  pyy (opt) integer pointer to get year (4 digits)
**  pmm (opt) integer pointer to get month
**  pdd (opt) integer pointer to get day of month
*/
void stlDateNow(int *pNr,int *pyy,int *pmm,int *pdd)
{
	stlDateTimeGetUTC(0,pNr,pyy,pmm,pdd,NULL,NULL,NULL,NULL,NULL);
}

/* get current date in various formats
**  pyy (opt) integer pointer to get hour
**  pmm (opt) integer pointer to get minute
**  pdd (opt) integer pointer to get second
*/
void stlTimeNow(int *phh,int *pmm,int *pss)
{
	stlDateTimeGetUTC(0,NULL,NULL,NULL,NULL,phh,pmm,pss,NULL,NULL);
}

/* Convert given date and time toe Julian date and time.
**  iYear    year 1900..2399
**  iMonth   month 1..12
**  iMday    day of the month 1..31
**  iHour    Hour 0..23
**  iMin     Minute 0..59
**  iSec     Seconds 0..59
** Return Julian date for given date and time
*/
double stlJulianDate(int iYear,int iMonth,int iMday,int iHour,int iMin,int iSec)
{
	double dJD=2439856.0;
	dJD+=stlDateToInt(iYear,iMonth,iMday);
	dJD+=((iHour*3600)+(iMin*60)+iSec)/86400.0;
	return dJD;
}
/* Get current Julian date and time
** Return current Julian date.time
*/
double stlJulianNow(void)
{
	int dYY,dMM,dDD,tHH,tMM,tSS;
	stlDateTimeGetUTC(0,NULL,&dYY,&dMM,&dDD,&tHH,&tMM,&tSS,NULL,NULL);
	return stlJulianDate(dYY,dMM,dDD,tHH,tMM,tSS);	
}

/* Convert Julian date to human year/month/day
**  pNr (opt) Integer representation of date (used in stlIntToDate and stlDateToInt)
**  pyy (opt) integer pointer to get year (4 digits)
**  pmm (opt) integer pointer to get month
**  pdd (opt) integer pointer to get day of month
*/
void stlJulian2Date(double dJD,int *pNr,int *pyy,int *pmm,int *pdd)
{
	int iDat=(int)(dJD-2439855.99999999999);
	if (pNr) *pNr=iDat;
	if ((pyy)||(pmm)||(pdd)) stlIntToDate(iDat,pyy,pmm,pdd);
}

/* Convert Julian date to human hour/min/sec
**  pyy (opt) integer pointer to get hour
**  pmm (opt) integer pointer to get minute
**  pdd (opt) integer pointer to get second
*/
void stlJulian2Time(double dJD,int *phh,int *pmm,int *pss)
{
	int iDat=(int)(dJD+0.0000000000001);
	dJD-=iDat;		// gedeelte achter de . blijft over
	iDat=(int)(dJD * 86400.000000000001);	// aantal seconden in deze dag
	if (phh) *phh=iDat/3600;
	if (pmm) *pmm=(iDat/60)%60;
	if (pss) *pss=iDat%60;
}
