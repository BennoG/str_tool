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
#include <stdio.h>
#include <string.h>
#include "wchar.h"

# define stricmp(a,b)    strcasecmp(a,b)
# define strcmpi(a,b)    strcasecmp(a,b)
# define strnicmp(a,b,c) strncasecmp(a,b,c)
# define _wcsnicmp(a,b,c) wcsncasecmp(a,b,c)
char *strupr (char *p);
char *strlwr (char *p);
wchar_t *wcsupr (wchar_t *p);
wchar_t *wcslwr (wchar_t *p);

int   atoi_b (const char *nptr,int base);
char *itoa   (int nr, char *buf, int base);

#ifdef printf
#  undef printf
#endif
#define printf stlPrintf

int stlPrintf(const char *format , ... );

struct stlLogInfoStruct
{
	STP sMsg;				// hier komt melding in te staan
	char sNam[50];			// naam van de thread
	int timH,timM,timS;		// huidige tijd
	int datY,datM,datD;		// huidige datum
	unsigned int msTime;	// bericht tijd in ms
	int threadID;			// thread ID
};

/* Set user handler for printf output of application
**  userHandler  function to be called on each printf
*/
void stlPrintfHandler(void (*userHandler)(STP));
void stlPrintfHandler2(void (*userHandler)(struct stlLogInfoStruct*));

// printf redirection in windows
void stlPrintfSetMsTimes(int iOn);
void stlPrintfSetLogFn(const char *sLogFn);


/* Do printf screen and file handling in a separate low priority thread
*/
void stlPrintfThread();

