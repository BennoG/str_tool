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
#ifndef _STR_TL_H_
#define _STR_TL_H_ 1

#include <stdarg.h>
#include <stdio.h>
#include "stl_thread.h"

#ifdef  __cplusplus
#  include <string>
extern "C" {
#endif

extern int _strTlMaxBufLen;	/* Never allocate more tan BufLen bytes */

struct str_data{
	char *sBuf;         /* Buffer to data */
	int  iLen;			/* Length of data */
	int  iLenLoc;		/* allocated length of data */
	int _uid;			/* Unique identity and check */
	stlMutex _mux;		/* Mutex to make var thread save */
};

typedef struct str_data * STP;

#if defined(__linux__) || defined(__APPLE__)
// include extra Linux functions
#  include "stl_strlnx.h"
#endif

#ifdef _WIN32
// include extra Windows functions
#  include "stl_strwin.h"
#endif

#define _StrDataUid_	0x5A1234A5

#define _D1 ((char)255)
#define _D2 ((char)254)
#define _D3 ((char)253)

#define _sD1 "\377"
#define _sD2 "\376"
#define _sD3 "\375"


#define _StTlErrMaxMemUsed_			1	// Max alowed memory used
#define _StTlErrOutOfMem_				2	// Out of memory
#define _StTlErrNullVar_				3	// Internal NULL pointer used
#define _StTlErrInvalVar_				4	// Invalid or overwriten var used
#define _StTlNoHighSpeedTimer_	5	// No highspeed timer support  (Windows only)
#define _StTlWinsockNotFound_		6	// No wsock 32 installed       (Windows only)
#define _StTlErrThreadMaxUsed_	7	// Max number of threads reached
#define _StTlErrTcpSrvBind_			8	// Tcp server bind socket errror
#define _StTlErrTcpSrvMemory_		9	// Tcp server out of memory

/* Function called when an emergency error occurred
 *	can be overwritten by application to generate custom errors
 */
void _strSafeExit(int iReasen);

/* Release memory used by struct
 */
void stlFree(STP pVA);

/* Create new STP data with iBytes length (excluding terminating 0)
 *	iBytes  numb of bytes space to alloc
 *  cFill   char to fill data with
 * Return STP data with 
 */
STP stlInitLen(int iBytes,char cFill);

/* Create new STP data with iBytes length (excluding terminating 0)
 *	iBytes  numb of bytes space to alloc
 *  cFill   char to fill data with
 * Return STP data with 
 */
STP stlInitLenSrc(int iBytes,char *src);

/* Initialize new struct ptr with string
 */
STP stlSetSt (const char *pStr);

/* Initialize new strict withe the same contents
 */
STP stlCopy(STP pVA);

/* Copy 1 field from source data into another field in destination data
 *	sDst  destination data
 *  iD1,iD2,iD3 Index numbers destination (neg values count from the back)
 *	sSrc  source data
 *  iS1,iS2,iS3 Index numbers source (neg values count from the back)
 */
void stlCpyVld(STP sDst,int iD1,int iD2,int iD3,STP sSrc,int iS1,int iS2,int iS3);

/* Initialize new struct ptr with string like printf 
 */
STP stlSetStf(const char *pStr,...);

/* Initialize new struct ptr with string with va_list arguments
 */
STP stlSetSta(const char *pStr,va_list args_lst);

/* Various ways to append data to existing string struct ptr
 */
void stlAppendSt (STP pVA,const char *pStr);
void stlAppendStf(STP pVA,const char *pStr,...);
void stlAppendCh (STP pVA,char ch);
void stlAppendStp(STP pVA,STP pAppend);

/* Search and replace characters in buf 
 *  pVa  data to convert chars in
 *  cSrc char to search for
 *  cRep char to replace with
 * Return number of replacements done
 */
int  stlConvert(STP pVA,char cSrc,char cRep);

/* exchange selected chars in al field
 *  cZoek character to search for
 *  cRepl character to replace it with
 *	sVal  data buffer
 *  iD1   index1  (neg values count from the back)
 *  iD2   index2  (neg values count from the back)
 *  iD3   index3  (neg values count from the back)
 * Return number of hits on search
 */
int stlConvertV(char cZoek,char cRepl,STP sVal,int iD1,int iD2,int iD3);

/* Rotate STP data array
*/
void stlRotate(STP sData);

/* Search for a substring replace with another string
 */
int  stlSwapStr(STP pVA,const char *sSrc,const char *sRep);

/* Exchange the content of two data blocks
 *	Useful if u want to replace all data in a block as function var.
 */
void stlExchange(STP sV1,STP sV2);

/* Get value from delimiter STP data
 * pVA  value to get data from
 * iD1  index1  (neg values count from the back)
 * iD2  index2  (neg values count from the back)
 * iD3  index3  (neg values count from the back)
 */
STP  stlGetFld(STP pVA,int iD1,int iD2,int iD3);
STP  stlGetStr(STP pVA,int iD1,int iD2,int iD3);	// Alias for stlGetFld
/* from stl_integer.c */
int  stlGetInt(STP pVA,int iD1,int iD2,int iD3);
unsigned int  stlGetHex (STP pVA,int iD1,int iD2,int iD3);
unsigned int  stlGetBin (STP pVA,int iD1,int iD2,int iD3);
/* from stl_double.c */
double stlGetDlmDouble(STP pVA,int iD1,char cDlm);
double stlGetDouble   (STP pVA,int iD1,int iD2,int iD3);
void stlStoreDlmDouble(double dVal,STP pVA,int iD1,char cDlm);
void stlStoreDouble   (double dVal,STP pVA,int iD1,int iD2,int iD3);
void stlInsertDouble  (double dVal,STP pVA,int iD1,int iD2,int iD3);

/* Get length of delimited STP sub part
**  pVA  value to get data from
**  iD1  index1  (neg values count from the back)
**  iD2  index2  (neg values count from the back)
**  iD3  index3  (neg values count from the back)
** Return length of sub part
*/
int stlGetLen(STP pVA,int iD1,int iD2,int iD3);

/* Get value from delimiter STP data into normal string (max length given)
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  sBuf buffer to write data in
 *	iLen max number of data to write (including terminating 0)
 * Return number of bytes written.
 */
int stlGetStN(STP pVA,int iD1,int iD2,int iD3,char *sBuf,int iLen);

/* Copy 1 field from source data into another field in destination data
 *	sDst  destination data
 *  iD1,iD2,iD3 Index numbers destination (neg values count from the back)
 *	sSrc  source data
 *  iS1,iS2,iS3 Index numbers source (neg values count from the back)
 */
void stlCpyVld(STP sDst,int iD1,int iD2,int iD3,STP sSrc,int iS1,int iS2,int iS3);

/* Get value from STP with user delimiter
 *	pVA  Value to get subpart of
 *  iD1  index to get (neg count from the back)
 *  cDlm character to use as delimiter
 * Return SubPart of STP
 */
STP stlGetDlm(STP pVA,int iD1,char cDlm);
int stlGetDlmInt(STP pVA,int iD1,char cDlm);
int stlGetDlmHex(STP pVA,int iD1,char cDlm);
int stlGetDlmIntAuto(STP pVA,int iD1,char cDlm);	// let op max 0x7FFFFFFF (signed)
int stlGetDlmIntAutoD(STP pVA,int iD1,char cDlm,int iDefault);	// let op max 0x7FFFFFFF (signed)


/* Get sub part from string (safe)
 *	pVA  data to get substring from
 *  iOfs offset from start (<0 offset from end)
 *  iLen number of chars to get.
 * Return SubPart of STP
 */
STP stlGetSect(STP pVA,int iOfs,int iLen);

/* Count number of selected chars in al field
 *	sVal  data buffer
 *  iD1   index1  (neg values count from the back)
 *  iD2   index2  (neg values count from the back)
 *  iD3   index3  (neg values count from the back)
 *  cDlm  character to search for
 * Return number of hits on search
 */
int stlCountV(STP sVal,int iD1,int iD2,int iD3,char ch);

/* Count number of occurrences of selected char in buffer
 *	sBuf STP data to count in
 *  ch   char to be counted
 * return number of occurrences.
 */
int stlCount(STP sV1,char ch);

/* Get index of nTh (1=first, 2 second, -1=last) occurrence of cDlm in the buffer
** Return -1 not found
**        -2 NULL data ptr
*/
int stlIndexOfN(STP sV1,char cDlm,int iD1);

/* Get index of first occurrence of cDlm in the buffer
** Return -1 not found
**        -2 NULL data ptr
*/
int stlIndexOf(STP sV1,char cDlm);

/* Insert or delete a number of characters at a specifiek position
 * This function is safe against mistakes
 *  pVA   STP data to delete or insert characters
 *  iPos  Position in dat to start at
 *  iLen  Number of bytes to delete (<0) or insert (>0)
 * Return New length of STP data
 */
int stlInsDel(STP pVA,int iPos,int iLen);

/* Store value into delimiter data
 *  pStr String to store in data
 *  pVA  STP struct to store data in
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlStoreStr(const char *pStr,STP pVA,int iD1,int iD2,int iD3);
/* from stl_integer.c */
void stlStoreInt(int iVal  ,STP pVA,int iD1,int iD2,int iD3);
void stlStoreHex(int iVal  ,STP pVA,int iD1,int iD2,int iD3);
void stlStoreBin(int iVal  ,STP pVA,int iD1,int iD2,int iD3);

/* Store value into delimited data
*  pStr String to store in data
*  pVA  STP struct to store data in
*  iD1  index1  (neg values count from the back)
*  cDlm char to use as delimiter
*/
void stlStoreDlm(const char *pStr,STP pVA,int iD1,char cDlm);
void stlStoreDlmInt(int iVal,STP pVA,int iD1,char cDlm);
void stlInsertDlm(const char *pStr,STP pVA,int iD1,char cDlm);

/* Insert value into delimiter data
 *  pStr String to store in data
 *  pVA  STP struct to insert data in
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlInsertStr(const char *pStr,STP pVA,int iD1,int iD2,int iD3);
/* from stl_integer.c */
void stlInsertInt(int iVal  ,STP pVA,int iD1,int iD2,int iD3);

/* Store value into delimiter data
 *  pVA  STP struct to store data in
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  pStr Formated String to store in data
 *  ...  optional parameters
 */
void stlStoreStrf(STP pVA,int iD1,int iD2,int iD3,const char *pStr,...);

/* Insert value into delimiter data
 *  pVA  STP struct to insert data in
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  pStr Formated String to insert in data
 *  ...  optional parameters
 */
void stlInsertStrf(STP pVA,int iD1,int iD2,int iD3,const char *pStr,...);

/* Locate a string in the data block
 *  sFind string to search for
 *  pVA   data to search in from
 *  iD1   index1  (neg values count from the back)
 *  iD2   index2  (neg values count from the back)
 *  piVld ptr to value where index is to be stored (optional)
 *  sOpts optional compare flags ("I"=case Insensitive, "N"=no Sorting, "R"=Right aligned "E"=Number aligned)
 * Return 1 when found (piVld is set to index)
 *        0 not found (piVld set to position to insert)
 */
int stlLocate(const char *sFind,STP pVA,int iD1,int iD2,int *piVld,const char *sOpts);

/* Locate a string in the data block
 *  sFind string to search for
 *  pVA   data to search in from
 *  cDlm  Char to use as delimiter
 *  piVld ptr to value where index is to be stored (optional)
 *  sOpts optional compare flags ("I"=case Insensitive, "N"=no Sorting, "R"=Right aligned "E"=Number aligned)
 * Return 1 when found (piVld is set to index)
 *        0 not found (piVld set to position to insert)
 */
int stlLocateDlm(const char *sFind,STP pVA,char cDlm,int *piVld,const char *sOpts);



/* Delete field in data
 *  pVA  value to delete field from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlDelStr(STP pVA,int iD1,int iD2,int iD3);

/* Delete field in data
 *  pVA  value to delete field from
 *  iD1  index1  (neg values count from the back)
 *  cDlm character to use as delimiter
 * Return SubPart of STP
 */
void stlDelDlm(STP pVA,int iD1,char cDlm);

/* Count number of occurrences of selected char in buffer
 *	sBuf buffer to string to count in
 *  ch   char to be counted
 * return number of occurrences.
 */
int stlCountChr(const char *sBuf,char ch);

/* Remove starting en tailing spaces from data
 *	pVA  data structure
 * Return new length of data
 */
int stlStrip(STP pVA);

/* Remove starting en tailing char from data
**	pVA    data structure
**  cStrip char to remove from start / end
** Return new length of data
*/
int stlStripChr(STP pVA,char cStrip);

/* Ensure only single occurrences of char
 *	pVA  data to make single occurrence
 *  ch   char te remove from data
 * Return new data length
 */
int stlSingle(STP pVA,char chr);

/* Remove all occurrences of char ch
 *	pVA  data to remove chars from
 *  ch   char te remove from data
 * Return new data length
 */
int stlRemove(STP pVA,char chr);

/* Remove all occurrences of chars
 *	pVA     data to remove chars from
 *  sRemChs chars te remove from data
 * Return new data length
 */
int stlRemoveMult(STP pVA,const char *sRemChs);

/* Leave all occurrences of chars
 *	pVA     data to leave chars in
 *  sRemChs chars te leve in the data
 * Return new data length
 */
int stlLeaveMult(STP pVA,const char *sHoldChs);

/* Read a file from some position with some length
 *	sFilename  Name of file to read
 *  iStart     >=0 start position where to start reading
 *             <0  Number of bytes from the end of the file (not for /proc/ on Linux)
 *  iBytes     <0  Read til end of file 
 *             >=0 Number of bytes to read from file
 * Return NULL file does not exist of acces deny
 *        STP  Data read from file
 */
STP stlFileRead(const char *sFilename,int iStart,int iBytes);

/* Test if file exists
*	sFilename  Name of file to read
*  Return 0 no file
*         1 yes file exists
*/
int stlFileExist(const char *sFileName);

/* Write data to file
 *  sData      data to write to file
 *  sFileName  name to use for file
 * Return 0 Ok
 *       -1 illegal filename
 *       -2 not all data could be written (disk full ??)
 */
int stlFileWrite(STP sData,const char *sFilename);

/* Write data to file in specific directory (create directory if not exists)
 *  sData      data to write to file
 *  sFileName  name to use for file
 * Return 0 Ok
 *       -1 illegal filename
 *       -2 not all data could be written (disk full ??)
 */
int stlFileWriteDir(STP sData,const char *sDir,const char *sFilename);

/* Create directory including previous directory's (like Linux mkdir -p)
*/
int stlMakeDirp(const char *sDirName);


/* Read string struct from FILE data stream
 *	fIn      communication structure
 *  sIgnore  string of characters to ignore in data stream.           (optional)
 *  sStop    string of chars to stop on.                              (optional)
 * return: NULL End of file reached
 *         Data data was read til stop char was reached
 */
STP stlFileReadLineEx(FILE *fIn,const char *sIgnore,const char *sStop);
STP stlFileReadLine(FILE *fIn);



/* Calculate CRC32 from char
 *	ch     char to update crc with
 *  crc    crc to update
 * Return new crc value
 */
unsigned long stlCrc32(unsigned char ch, unsigned long crc);

/* Calculate CRC32 from file
 *	name    name fo file to calculate crc from
 *  crc     (opt) pointer long int for crc value
 *  charcnt (opt) pointer to integer receiving number of bytes in file
 * Return 0 Ok
 *        2 file dos not exist
 *        3 File not fully read
 */
int stlCrc32File(const char *name, unsigned long *crc, long *charcnt);

/* Calculate CRC32 in memory
 *	buf   pointer to memory region
 *  len   number of bytes in buffer
 * Return crc of buffer
 */
unsigned long stlCrc32Mem    (const void *buf, size_t len);
unsigned long stlCrc32MemMPG (const void *buf, size_t len);

unsigned long stlCrc32Stp(STP sData);

/* Calculate CRC32 in memory multiple chunks
 *	buf    pointer to memory region
 *  len    number of bytes in buffer
 *  oldCrc (0 for first chunk) other chunks result of each one
 * Return crc of buffer
 */
unsigned long stlCrc32MemU(const void *vbuf, size_t len, unsigned long oldCrc);

/* Calculate md5 from memory buffer
 *	sBuf    pointer to buffer
 *  iBufLen number of bytes in buffer
 * Return md5Sum
 */
STP stlMd5Mem(const char *sBuf,int iBufLen);
STP stlMd5Stp(STP sData);

/* Init shared memory region
 *	sIdName    name to identify this shared memory
 *  iSize      Number of bytes to allocate
 * Return NULL Error
 *        ptr  pointer to allocated memory
 */
void *stlShmemGet(const char *sIdName,int iSize);

/* Release shared memory region
 *	sIdName    name to identify this shared memory
 */
void stlShmemRelease(const char *sIdName);


/************************************************************************/
/* Dat and time functions                                               */
/************************************************************************/


/* Convert date to integer
 *  yy  year (4 digits if < 100 then 2000 is added to year)
 *  mm  month
 *  dd  day of month
 * Return number of day since 1 Jan 1968
 */
int stlDateToInt(int yy,int mm,int dd);

/* Convert integer date back to actual date base date = 1 Jan 1968
 *  day Integer representation of date
 *  pyy (opt) integer pointer to get year (4 digits)
 *  pmm (opt) integer pointer to get month
 *  pdd (opt) integer pointer to get day of month
 */
void stlIntToDate(int day,int *pyy,int *pmm,int *pdd);

/* Calculate weekday from integer date.
 *  iDate date from stlDateToInt
 * Return (1=Sun 2=Mon 3=Tue 4=Wed 5=Thu 6=Fri 7=Sat)
 */
int stlIntTowWeekday(int iDate);

/* Calculate date from week number (customizable)
*  iWeek     Week number YYYYWW or WW
*  iFirstDay First day of the week (1-7) (2=Monday default)(0)
*  iMinDays  Minimum number of days in first week of a year (4=default)(0)
* Return Date in integer format
*/
int stlWeekToDateEx(int iWeek,int iFirstDay,int iMinDays);

/* Calculate year week number from specific date (customizable)
 *  yy        Year (in 4 digits)
 *  mm        Month
 *  dd        Date
 *  iFirstDay First day of the week (1-7) (2=Monday default)
 *  iMinDays  Minimum number of days in first week of a year (4=default)
 * Return
 *  YYYYWW
 */
int stlDateToWeekEx(int yy,int mm,int dd,int iFirstDay,int iMinDays);

/* Calculate year week number from specific date
 *  yy        Year (in 4 digits)
 *  mm        Month
 *  dd        Date
 * Return
 *  YYYYWW
 */
int stlDateToWeek(int yy,int mm,int dd);

/* get current date in various formats
**  pNr (opt) Integer representation of date (used in stlIntToDate and stlDateToInt)
**  pyy (opt) integer pointer to get year (4 digits)
**  pmm (opt) integer pointer to get month
**  pdd (opt) integer pointer to get day of month
*/
void stlDateNow(int *pNr,int *pyy,int *pmm,int *pdd);

/* get current date in various formats
**  pyy (opt) integer pointer to get hour
**  pmm (opt) integer pointer to get minute
**  pdd (opt) integer pointer to get second
*/
void stlTimeNow(int *phh,int *pmm,int *pss);

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
void stlDateTimeGetUTC(int iUtcFlg,int *pDayNr,int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond,int *pMsec,int *tzOffset);

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
int stlDateTimeSet(int iYear,int iMonth,int iDay,int iHour,int iMinute,int iSecond);

/* Get time of operating system (local time zone)
**  pYear   (opt) year
**  pMont   (opt) month
**  pDay    (opt) Day of month
**  pHour   (opt) time hour
**  pMinute (opt) time minute
**  pMinute (opt) time second
** Return 0 Ok
*/
void stlDateTimeGet(int *pYear,int *pMonth,int *pDay,int *pHour,int *pMinute,int *pSecond);

/* Convert given date and time toe Julian date and time.
**  iYear    year 1900..2399
**  iMonth   month 1..12
**  iMday    day of the month 1..31
**  iHour    Hour 0..23
**  iMin     Minute 0..59
**  iSec     Seconds 0..59
** Return Julian date for given date and time
*/
double stlJulianDate(int iYear,int iMonth,int iMday,int iHour,int iMin,int iSec);

/* Get current Julian date and time
** Return current Julian date.time
*/
double stlJulianNow(void);

/* Convert Julian date to human year/month/day
**  pNr (opt) Integer representation of date (used in stlIntToDate and stlDateToInt)
**  pyy (opt) integer pointer to get year (4 digits)
**  pmm (opt) integer pointer to get month
**  pdd (opt) integer pointer to get day of month
*/
void stlJulian2Date(double dJD,int *pNr,int *pyy,int *pmm,int *pdd);

/* Convert Julian date to human hour/min/sec
**  phh (opt) integer pointer to get hour
**  pmm (opt) integer pointer to get minute
**  pss (opt) integer pointer to get second
*/
void stlJulian2Time(double dJD,int *phh,int *pmm,int *pss);


#ifdef _DEBUG
// Break on STL init alt specifiek length
void stlAllocBreak(int iLen);

// Start monitoring memory allocs
void stlAllocMon(void);

// Save all allocated memory to file
void stlAllocSave(const char *sFileName);
#endif


/* Change into a daemon. (Linux printf to syslog) (windows printf to messages)
*/
void stlDeamonize(void);

#ifdef _WIN32
  HWND stlGetConsoleHwnd(void);
#endif


/* sets system wide lock so application can only be running once
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int setPidLock(const char *_LockFile_);

/* Test of application already running
**  _LockFile_   complete path to where the lock is located (linux style)
*/
int testPidLock(const char *_LockFile_);


#define _RandCapital_	0x01	// A-Z
#define _RandLower_		0x02	// a-z
#define _RandNumber_	0x04	// 0-9
#define _RandPrint_		0x08	// char(32-127)
#define _RandNoZero_	0x10	// char(1-255)
#define _RandAllChar_	0x20	// char(0-255)
/* Generate reproducible random data
**  iLen    number of data bytes to generate
**  iMode   Type of data to generate (flags can be or'd together)
**  uiSeed1 random seed number 1
**  uiSeed2 random seed number 2
**  uiSeed3 random seed number 3
** Return Resulting random data
*/
STP stlRandom(int iLen,int iMode,unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3);


/* Spawn an other proces an redirect stdout and stderr to user definable functions.
**  sCommand   command to execute including parameters
**  iWait      (1)wait for the child proces to exit (return == exit value of child)  (0)return immediately
**  fxOut      function to be called when child writes to stdout (optional)
**  pUserOut   data pointer witch is passed to fxOut
**  fxErr      function to be called when child writes to stderr (optional)
**  pUserErr   data pointer witch is passed to fxErr
** Return -1   unable to create pipe
**        -2   unable to fork current proces
**       >=0   Child proces exit status (if iWait set) or 0 (if iWait = 0)
*/
int stlExecPipe(char *sCommand,int iWait,void(*fxOut)(char *sData,int iLen,void *pUserOut),void *pUserOut,void(*fxErr)(char *sData,int iLen,void *pUserErr),void *pUserErr);


/* Get info from .ini file
** sSection main part of ini file
** sItem    (optional) subitem in section of ini
**          when empty or NULL the whole section is returned
*/
STP stlIniGet(const char *sSection,const char *sItem);

/* Get file name of .ini file
** sSection main part of ini file
** sItem    (optional) subitem in section of ini
**          when empty or NULL the whole section is returned
*/
const char *stlIniName();
void stlIniNameSet(const char *name);


/* Get info from .ini file
** sSection  main part of ini file
** sItem     subitem in section of ini
** sDefault  Default value if not in .ini file or empty
*/
STP stlIniGetD(const char *sSection,const char *sItem,const char *sDefault);
int stlIniGetIntD(const char *sSection,const char *sItem,int iDefault);


/* Set info into .ini file
**  sSection main part of ini file
**  sItem    subitem in section of ini
**  sValue   new value for item (NULL remove item)
** Return 0 Ok
**       -1 Section part is invalid
**       -2 item part is invalid
*/
int stlIniSet(const char *sSection,const char *sItem,const char *sValue);

STP stlUrlEncode(const char *src);
STP stlUrlDecode(const char *src);

/* Get string part of STP and releases memory of STP
** warning max 15 concurrent string parts allowed
const char *stlStringConsume(STP sValue);
*/

/* Get string part of STP and releases memory of STP
** warning max 15 concurrent string parts allowed
** in case of NULL STP the result is "" and not NULL
const char *stlStringConsumeNN(STP sValue);
*/


#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus

#define _StrDataUidC_	0x5A1234A6
//#define _strTlMaxBufLen	11024000

extern bool ansStlDebug;

namespace ansStl
{
	class cST
	{
	private:
		void initVars(){sBuf = NULL, sWBuf = NULL; iLen = iLenLoc = 0; _uid = _StrDataUidC_; isUTF8 = false;}
		char  *sBuf;		/* Buffer to data */
		wchar_t *sWBuf;     /* Buffer to data */
		int  iLen;			/* Length of data */
		int  iLenLoc;		/* allocated length of data */
		int _uid;			/* Unique identity and check */
	public:
		bool isUTF8;		// when true 8bit strings are interpreted as it they are UTF-8
	public:
		cST();
		cST(STP p,bool consume = false);
		cST(int len,const char *p);
		cST(int len,const wchar_t *p);
		cST(const wchar_t *p,...);
		cST(const char  *p,...);
		cST(va_list ar,const char *p);
		cST(va_list ar,const wchar_t *p);
		cST(const cST &src);
		cST(const std::string &src);
		~cST();
		/* Insert or delete a number of characters at a specifiek position
		 * This function is safe against mistakes
		 *  iPos  Position in dat to start at
		 *  iLen  Number of bytes to delete (<0) or insert (>0)
		*/
		void insDel(int iPos,int iLen);
		int  length(){return iLen;}
		STP  getStp();
		const char *buf();
		const wchar_t *bufW();
		void set(STP p,bool consume = false);
		void set(int len,const wchar_t *p);
		void set(int len,const char  *p);
		void set(int len,const void  *p){set(len,(const char*) p);}
		void set(const cST *val);
		void set(const cST &val){set(&val);}
		void set(const std::string &val);
		void set(const char *val);
		void set(const wchar_t *val);

		void setf(const wchar_t *p,...);
		void setf(const char  *p,...);
		void setA(va_list ar,const char *p);
		void setA(va_list ar,const wchar_t *p);
		void append(cST &p);
		void append(cST *p);
		void append(STP p, bool consume = false);
		void append(int len,const wchar_t *p);
		void append(int len,const char  *p);
		void append(int len,const void  *p){append(len,(const char*)p);}
		void append(const wchar_t *p,...);
		void append(const char  *p,...);
		void appendA(va_list ar,const char *p);
		void appendA(va_list ar,const wchar_t *p);
		void append(wchar_t ch){wchar_t s[]={ch,0}; append(1,s);}
		void append(char ch){char s[]={ch,0}; append(1,s);}
		void insert(int iPos,const char *fmt,...);
		int  convert(int chZoek,int chRep);
		int  convert(const char *sZoek,const char *sRep);
		void strip(int cStrip = ' ');

		/* Ensure only single occurrences of char
		 *  ch   char te remove from data
		 * Return new data length
		 */
		int single(char chr);
		int single(wchar_t chr);

		/* Remove all occurrences of char ch
		 *  ch   char te remove from data
		 * Return new data length
		 */
		int remove(char chr);
		int remove(wchar_t chr);

		/* Remove all occurrences of chars
		 *  sRemChs chars te remove from data
		 * Return new data length
		 */
		int removeMult(const char *sRemChs);

		/* Leave all occurrences of chars
		 *  sRemChs chars te leve in the data
		 * Return new data length
		 */
		int leaveMult(const char *sHoldChs);

		int  count(wchar_t cDlm);
		void toUpper();
		void toLower();

		bool StartsWith(const char *scmp,bool caseSensitive = true);
		bool StartsWith(const wchar_t *scmp,bool caseSensitive = true);
		bool EndsWith(const char *scmp,bool caseSensitive = true);
		bool EndsWith(const wchar_t *scmp,bool caseSensitive = true);
		bool contains(const char *sZoek);
		bool compare(ansStl::cST &scmp);
		bool compare(ansStl::cST *scmp);
		bool compare(const char *scmp);
		bool compare(const char *scmp,size_t len);
		bool comparei(const char *scmp);
		bool compareMatch(const char *match);		// compare like dos directory e.g. ben* of b?n*
		bool compareMatchI(const char *match);		// compare like dos directory e.g. ben* of b?n*

		/* Get value with user delimiter
		 *  iD1  index to get (neg count from the back)
		 *  cDlm character to use as delimiter
		 * Return SubPart of STP
		 */
		cST  getDlm(int iD1,wchar_t cDlm);
		int    toInt();
		double toDouble();
		int    getDlmInt(int iD1,wchar_t cDlm);
		double getDlmDbl(int iD1,wchar_t cDlm);
		void setDlm(int iD1,wchar_t cDlm,const char *fmt,...);
		void setDlm(int iD1,wchar_t cDlm,int iValue);
		void setDlm(int iD1,wchar_t cDlm,STP value,bool consume = false);
		void setDlm(int iD1,wchar_t cDlm,cST* value);
		void setDlm(int iD1,wchar_t cDlm,cST& value){setDlm(iD1,cDlm,&value);}
		void delDlm(int iD1,wchar_t cDlm);
		// test if 16 bit data is used
		bool is16bit(){return (sWBuf != NULL);}
		bool is8bit(){return (sBuf != NULL);}

		operator const char*();
		operator const char*() const;
		operator const unsigned char*();
		operator const unsigned char*() const;

		//cST& operator=(char* szSrc);
		cST& operator=(const char* szSrc);
		cST& operator=(const cST &strSrc);
		cST& operator=(STP sVal);

		cST& operator+=(const char* szSrc);
		cST& operator+=(const char cSrc);
		cST& operator+=(const cST &strSrc);

		cST operator+(const char* szSrc);

		char& operator[](int iIdx);

	private:
		void update8bit();
		void update16bit();
		void switch8bit();
		void switch16bit();
	};

	class stlResource
	{
	private:
		ansStl::cST resData;
	public:
		stlResource(){load();}
		ansStl::cST get(const char *name);
	private:
		void load();
	};
}

void stlFree(STP sVa1,STP sVa2);
void stlFree(STP sVa1,STP sVa2,STP sVa3,STP sVa4 = NULL);
void stlFree(STP sVa1,STP sVa2,STP sVa3,STP sVa4,STP sVa5,STP sVa6 = NULL);

#endif

#endif

