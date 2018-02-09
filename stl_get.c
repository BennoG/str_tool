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

/* Get value from delimited STP data
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 * Return STP with requested subpart of data
 */
STP stlGetFld(STP pVA,int iD1,int iD2,int iD3)
{
	STP pRes;
	int iStart=0,iEind;
	if (pVA==NULL) return stlSetSt("");
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind);
	_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind);
	_strTlGetOfs(pVA->sBuf,_D3,iD3,&iStart,&iEind);

	pRes=_strTlInitVar(iEind-iStart);
	if (iEind>iStart) memcpy(pRes->sBuf,pVA->sBuf+iStart,iEind-iStart);
	_strTlUnlockVar(pVA);

	return pRes;
}
STP stlGetStr(STP pVA,int iD1,int iD2,int iD3)
{
	return stlGetFld(pVA,iD1,iD2,iD3);
}
/* Get length of delimited STP sub part
**  pVA  value to get data from
**  iD1  index1  (neg values count from the back)
**  iD2  index2  (neg values count from the back)
**  iD3  index3  (neg values count from the back)
** Return length of sub part
*/
int stlGetLen(STP pVA,int iD1,int iD2,int iD3)
{
	int iRes=0;
	STP sRes=stlGetFld(pVA,iD1,iD2,iD3);
	if (sRes) iRes=sRes->iLen;
	stlFree(sRes);
	return iRes;
}


/* Get value from delimterd STP data into normal string (max lengtht given)
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 *  sBuf buffer to write data in
 *	iLen max number of data to write (including terminating 0)
 * Return number of bytes written.
 */
int stlGetStN(STP pVA,int iD1,int iD2,int iD3,char *sBuf,int iLen)
{
	int iStart=0,iEind;
	if (sBuf==NULL) return 0;
	if (pVA==NULL){sBuf[0]=0;return 0;}
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind);
	_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind);
	_strTlGetOfs(pVA->sBuf,_D3,iD3,&iStart,&iEind);

	// Misuse iEind as length counter
	iEind-=iStart;
	if (iEind>iLen-1) iEind=iLen-1;
	if (iEind<0) iEind=0;
	if (iEind>0) memcpy(sBuf,pVA->sBuf+iStart,iEind);
	_strTlUnlockVar(pVA);
	sBuf[iEind]=0;
	return iEind;
}


/* Get value from STP with user delimiter
 *	pVA  Value to get subpart of
 *  iD1  index to get (neg count from the back)
 *  cDlm character to use as delimiter
 * Return SubPart of STP
 */
STP stlGetDlm(STP pVA,int iD1,char cDlm)
{
	STP pRes;
	int iStart=0,iEind;
	if (pVA==NULL) return stlSetSt("");
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	_strTlGetOfs(pVA->sBuf,cDlm,iD1,&iStart,&iEind);

	pRes=_strTlInitVar(iEind-iStart);
	if (iEind>iStart) memcpy(pRes->sBuf,pVA->sBuf+iStart,iEind-iStart);
	_strTlUnlockVar(pVA);

	return pRes;
}
/* Get sub part from string (safe)
 *	pVA  data to get substring from
 *  iOfs offset from start (<0 offset from end)
 *  iLen number of chars to get.
 * Return SubPart of STP
 */
STP stlGetSect(STP pVA,int iOfs,int iLen)
{
	STP pRes;
	if (pVA==NULL) return stlSetSt("");
	_strTlLockVar(pVA);
	if (iOfs<0) iOfs=pVA->iLen-iOfs;				// Calculate offset from end
	if (iOfs>pVA->iLen) iOfs=pVA->iLen;			// Prevent reading past end of data
	if (iLen+iOfs>pVA->iLen) iLen=pVA->iLen-iOfs;	// Prevent endpart past end of data.
	pRes=_strTlInitVar(iLen);
	if (iLen) memcpy(pRes->sBuf,&pVA->sBuf[iOfs],iLen);
	_strTlUnlockVar(pVA);

	return pRes;
}

/* Count number of selected chars in al field
 *	sVal  data buffer
 *  iD1   index1  (neg values count from the back)
 *  iD2   index2  (neg values count from the back)
 *  iD3   index3  (neg values count from the back)
 *  cDlm  character to search for
 * Return number of hits on search
 */
int stlCountV(STP sVal,int iD1,int iD2,int iD3,char cDlm)
{
	int iStart=0,iEind,i,iHit=0;
	if (sVal==NULL) return 0;
	_strTlLockVar(sVal);
	iEind=sVal->iLen;
	_strTlGetOfs(sVal->sBuf,_D1,iD1,&iStart,&iEind);
	_strTlGetOfs(sVal->sBuf,_D2,iD2,&iStart,&iEind);
	_strTlGetOfs(sVal->sBuf,_D3,iD3,&iStart,&iEind);
	for (i=iStart;i<iEind;i++){
		if (sVal->sBuf[i]==cDlm) 
			iHit++;
	}
	_strTlUnlockVar(sVal);
	return iHit;
}

/* exchange selected chars in al field
 *  cZoek character to search for
 *  cRepl character to replace it with
 *	sVal  data buffer
 *  iD1   index1  (neg values count from the back)
 *  iD2   index2  (neg values count from the back)
 *  iD3   index3  (neg values count from the back)
 * Return number of hits on search
 */
int stlConvertV(char cZoek,char cRepl,STP sVal,int iD1,int iD2,int iD3)
{
	int iStart=0,iEind,i,iHit=0;
	if (sVal==NULL) return 0;
	_strTlLockVar(sVal);
	iEind=sVal->iLen;
	_strTlGetOfs(sVal->sBuf,_D1,iD1,&iStart,&iEind);
	_strTlGetOfs(sVal->sBuf,_D2,iD2,&iStart,&iEind);
	_strTlGetOfs(sVal->sBuf,_D3,iD3,&iStart,&iEind);
	for (i=iStart;i<iEind;i++){
		if (sVal->sBuf[i]==cZoek){
			sVal->sBuf[i]=cRepl;
			iHit++;
		}
	}
	_strTlUnlockVar(sVal);
	return iHit;
}
