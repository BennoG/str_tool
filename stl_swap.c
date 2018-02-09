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

// Convert cScr to cRep in data
// Return number of replacemens maked
int stlConvert(STP pVA,char cSrc,char cRep)
{
	int iHit,i;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	for (iHit=i=0;i<pVA->iLen;i++){
		if (pVA->sBuf[i]==cSrc){
			pVA->sBuf[i]=cRep;
			iHit++;
		}
	}
	_strTlUnlockVar(pVA);
	return iHit;
}
int stlSwapStr(STP pVA,const char *sSrc,const char *sRep)
{
	int iSrc,iRep;
	int iHit=0,i;
	if ((pVA==NULL)||(sSrc==NULL)||(sRep==NULL)) return 0;
	iSrc=strlen(sSrc);
	iRep=strlen(sRep);
	if (iSrc==0) return 0;					// We can't look for nothing.
	_strTlLockVar(pVA);
	if (iSrc>=iRep){						// We don't need exta room
		char *pS,*pR;
		pS=pR=pVA->sBuf;					// data buffer
		for (i=pVA->iLen;i>0;i--){
			// memcmp use DWORD if len >= 4 so read could be beyond buffer boundry
			if ((i>=iSrc)&&(memcmp(sSrc,pS,iSrc)==0)){	// Found what we are looking for
				memcpy(pR,sRep,iRep);		// Swap new data in place
				pS+=iSrc;
				pR+=iRep;
				i -=iSrc-1;
				iHit++;
				continue;
			}
			*pR++=*pS++;
		}
		*pR=0;
		pVA->iLen -= (iHit*(iSrc-iRep));	// Calculate new data lenth
	}else{									// We need to expand the data
		char *pS,*pR;
		pS=pR=pVA->sBuf;					// data buffer
		for (i=pVA->iLen;i>0;i--){			// Lets count the number of times we have a hit
			if ((i>=iSrc)&&(memcmp(sSrc,pS,iSrc)==0)){	// Found what we are looking for
				pS+=iSrc;
				i -=iSrc-1;
				iHit++;
				continue;
			}
			pS+=1;
		}
		if (iHit>0){
			i=pVA->iLen;
			_strTlInsDel(pVA,0,iHit*(iRep-iSrc));	// Make room at beginning of data
			pR=pVA->sBuf;						// data buffer
			pS=pR+(iHit*(iRep-iSrc));			// Start offset to search
			for (;i>0;i--){
				if ((i>=iSrc)&&(memcmp(sSrc,pS,iSrc)==0)){	// Found what we are looking for
					memcpy(pR,sRep,iRep);		// Swap new data in place
					pS+=iSrc;
					pR+=iRep;
					i -=iSrc-1;
					continue;
				}
				*pR++=*pS++;
			}
			*pR=0;
		}
	}
	_strTlUnlockVar(pVA);
	return iHit;
}
