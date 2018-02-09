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
#include <stdlib.h>
#include <string.h>

#include "stl_str.h"
#include "internal.h"


/* Delete field in data
 *  pVA  value to delete field from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlDelStr(STP pVA,int iD1,int iD2,int iD3)
{
	int iStart=0,iEind;
	if (pVA==NULL) return;
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	if (_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind)){_strTlUnlockVar(pVA);return;}	// not found
	if (_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind)){_strTlUnlockVar(pVA);return;}	// not found
	if (_strTlGetOfs(pVA->sBuf,_D3,iD3,&iStart,&iEind)){_strTlUnlockVar(pVA);return;} // not found
	if (iD3){
		if ((iStart>0)&&(pVA->sBuf[iStart-1]==_D3)) iStart--;	else iEind++; // check to see if prev or next char need to be deleted
	} else if (iD2){
		if ((iStart>0)&&(pVA->sBuf[iStart-1]==_D2)) iStart--;	else iEind++; // check to see if prev or next char need to be deleted
	} else if (iD1){
		if ((iStart>0)&&(pVA->sBuf[iStart-1]==_D1)) iStart--;	else iEind++; // check to see if prev or next char need to be deleted
	}
	_strTlInsDel(pVA,iStart,iStart-iEind);
	_strTlUnlockVar(pVA);
}

/* Delete field in data
 *  pVA  value to delete field from
 *  iD1  index1  (neg values count from the back)
 *  cDlm character to use as delimiter
 * Return SubPart of STP
 */
void stlDelDlm(STP pVA,int iD1,char cDlm)
{
	int iStart=0,iEind;
	if (pVA==NULL) return;
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	if (_strTlGetOfs(pVA->sBuf,cDlm,iD1,&iStart,&iEind)){_strTlUnlockVar(pVA);return;}	// not found
	if (iD1){
		if ((iStart>0)&&(pVA->sBuf[iStart-1]==cDlm)) iStart--;	else iEind++; // check to see if prev or next char need to be deleted
	}
	_strTlInsDel(pVA,iStart,iStart-iEind);
	_strTlUnlockVar(pVA);
}

