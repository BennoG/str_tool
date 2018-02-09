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

/* Remove starting en tailing char from data
**	pVA    data structure
**  cStrip char to remove from start / end
** Return new length of data
*/
int stlStripChr(STP pVA,char cStrip)
{
	int i,j,k=0;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (i=j=0;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		if (ch==cStrip) continue;
		break;
	}
	for (;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		pVA->sBuf[j++]=ch;
		if (ch==cStrip) continue;
		k=j;		// First filtered char
	}
	pVA->iLen=k;
	pVA->sBuf[k]=0;
	_strTlUnlockVar(pVA);
	return k;
}
/* Remove starting en tailing spaces from data
**	pVA  data structure
** Return new length of data
*/
int stlStrip(STP pVA)
{
	return stlStripChr(pVA,' ');
}

/* Ensure only single occurrences of char
 *	pVA  data to make single occurrence
 *  ch   char te remove from data
 * Return new data length
 */
int stlSingle(STP pVA,char chr)
{
	int i,j,hit=0;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (i=j=0;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		if (ch==chr){
			if (hit) continue;	// We have had one
			hit=1;							// First one
		}else{
			hit=0;							// Reset hit flag
		}
		pVA->sBuf[j++]=ch;
	}
	pVA->iLen=j;
	pVA->sBuf[j]=0;
	_strTlUnlockVar(pVA);
	return j;
}

/* Remove all occurrences of char ch
 *	pVA  data to remove chars from
 *  ch   char te remove from data
 * Return new data length
 */
int stlRemove(STP pVA,char chr)
{
	int i,j;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (i=j=0;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		if (ch==chr) continue;
		pVA->sBuf[j++]=ch;
	}
	pVA->iLen=j;
	pVA->sBuf[j]=0;
	_strTlUnlockVar(pVA);
	return j;
}

/* Remove all occurrences of chars
 *	pVA     data to remove chars from
 *  sRemChs chars te remove from data
 * Return new data length
 */
int stlRemoveMult(STP pVA,const char *sRemChs)
{
	int i,j;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (i=j=0;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		if (strchr(sRemChs,ch)) continue;
		pVA->sBuf[j++]=ch;
	}
	pVA->iLen=j;
	pVA->sBuf[j]=0;
	_strTlUnlockVar(pVA);
	return j;
}

/* Leave all occurrences of chars
 *	pVA     data to leave chars in
 *  sRemChs chars te leve in the data
 * Return new data length
 */
int stlLeaveMult(STP pVA,const char *sHoldChs)
{
	int i,j;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (i=j=0;i<pVA->iLen;i++){
		char ch=pVA->sBuf[i];
		if (strchr(sHoldChs,ch)) 
			pVA->sBuf[j++]=ch;		// char may stay
	}
	pVA->iLen=j;
	pVA->sBuf[j]=0;
	_strTlUnlockVar(pVA);
	return j;
}

