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

#include "internal.h"
#include "stl_str.h"


void stlAppendSt (STP pVA,const char *pStr)
{
	int iPos;
	if ((pVA==NULL)||(pStr==NULL)) return;
	_strTlLockVar(pVA);
	iPos=pVA->iLen;
	_strTlInsDel(pVA,iPos,(int)strlen(pStr));
	strcpy(pVA->sBuf+iPos,pStr);
	_strTlUnlockVar(pVA);
}
void stlAppendCh (STP pVA,char ch)
{
	int iPos;
	if (pVA==NULL) return;
	_strTlLockVar(pVA);
	iPos=pVA->iLen;
	_strTlInsDel(pVA,iPos,1);
	pVA->sBuf[iPos]=ch;
	_strTlUnlockVar(pVA);
}
void stlAppendStp(STP pVA,STP pAppend)
{
	int iPos;
	if ((pVA==NULL)||(pAppend==NULL)) return;
	_strTlLockVar(pVA);
	_strTlLockVar(pAppend);
	iPos=pVA->iLen;
	_strTlInsDel(pVA,iPos,pAppend->iLen);
	memcpy(pVA->sBuf+iPos,pAppend->sBuf,pAppend->iLen);
	_strTlUnlockVar(pAppend);
	_strTlUnlockVar(pVA);
}
