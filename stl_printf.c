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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "internal.h"
#include "stl_str.h"

#ifdef _WIN32
#  ifndef va_copy
#     define va_copy(a,b) (a=b)
#  endif // va_copy
#endif

#ifdef __linux__
#  ifdef __GLIBC_PREREQ
#  else
#    define va_copy(a,b) (a=b)
#  endif
#endif

STP stlSetSta(const char *pStr,va_list args_lst)
{
	STP pVA;
	va_list args;
	int iLen=256;
	int iRes;
	while (1){
		va_copy(args,args_lst);
		pVA  = _strTlInitVar(iLen);
#if defined(_WIN32)
		iRes=_vsnprintf(pVA->sBuf,pVA->iLen-20,pStr,args);
#elif defined(__linux__)
		iRes=vsnprintf(pVA->sBuf,pVA->iLen-20,pStr,args);
#else
#  error unsupported os
#endif
		if (iRes >= (pVA->iLen-20)){iLen=iRes; iRes=-1;}	// C99 geeft aan dat benodigde lengte hier word weergegeven
		if (iRes>=0){pVA->iLen=iRes; return pVA;}
		stlFree(pVA);
		iLen=iLen+256+(iLen>>4);
		if ((_strTlMaxBufLen>0)&&(iLen>_strTlMaxBufLen)) _strSafeExit(_StTlErrMaxMemUsed_);
	}
}

STP stlSetStf(const char *pStr,...)
{
	STP pVA;
	va_list ap;

	va_start(ap,pStr);
	pVA=stlSetSta(pStr,ap);
	va_end(ap);
	return pVA;
}

void stlAppendStf(STP pVA,const char *pStr,...)
{
	STP pV1;
	va_list ap;
	if (pVA==NULL) return;
	va_start(ap,pStr);
	pV1=stlSetSta(pStr,ap);
	va_end(ap);
	stlAppendStp(pVA,pV1);
	stlFree(pV1);
}

/* Store value into delimiterd data
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  pStr Formated String to store in data
 *  ...  optional parameters
 */
void stlStoreStrf(STP pVA,int iD1,int iD2,int iD3,const char *pStr,...)
{
	STP pV1;
	va_list ap;
	if (pVA==NULL) return;
	va_start(ap,pStr);
	pV1=stlSetSta(pStr,ap);
	va_end(ap);
	stlStoreStr(pV1->sBuf,pVA,iD1,iD2,iD3);
	stlFree(pV1);
}

/* Insert value into delimiterd data
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  pStr Formated String to insert in data
 *  ...  optional parameters
 */
void stlInsertStrf(STP pVA,int iD1,int iD2,int iD3,const char *pStr,...)
{
	STP pV1;
	va_list ap;
	if (pVA==NULL) return;
	va_start(ap,pStr);
	pV1=stlSetSta(pStr,ap);
	va_end(ap);
	stlStoreStr(pV1->sBuf,pVA,iD1,iD2,iD3);
	stlFree(pV1);
}
