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

#define _FgNoCase_	0x0001
#define _FgRight_		0x0002
#define _FgDescend_	0x0004
#define _FgNoSort_	0x0008
#define _FgDecimal_	0x0010

#define _min(a,b)  (((a)<(b))?a:b)
#define _max(a,b)  (((a)>(b))?a:b)

static int _strFlgCmp(const char *sFind,int iZkLe,const char *sData,int iDaLe,int iFlgs)
{
	int iRes;
	if (iFlgs & _FgDecimal_)
	{
		if (iZkLe<iDaLe)
		{
			iRes=-1;
		}else if (iZkLe>iDaLe)
		{
			iRes=1;
		}else
		{
			iRes=memcmp(sFind,sData,iZkLe);
		}
	}else
	{
		if (iFlgs & _FgRight_)
		{
			if (iZkLe>iDaLe) sFind+=(iZkLe-iDaLe);
			if (iDaLe<iZkLe) sData+=(iDaLe-iZkLe);
		}
		if (iFlgs & _FgNoCase_) 
			iRes=strnicmp(sFind,sData,_min(iZkLe,iDaLe));
		else
			iRes=memcmp(sFind,sData,_min(iZkLe,iDaLe));
		if (iRes==0)
		{
			if (iZkLe>iDaLe) iRes= 1;
			if (iZkLe<iDaLe) iRes=-1;
		}
	}
	if ((iFlgs & _FgNoSort_)&&(iRes<0)) iRes=1;
	if (iFlgs & _FgDescend_) return -iRes;
	return iRes;
}

static int _strLocate(const char *sFind,const char *sBuf,int iStart,int iEind,char cDlm,int *piVld,const char *sOpts)
{
	int iFlgs=0,iPos,iRes,iZkLe,iVld;
	while ((sOpts)&&(*sOpts))
	{
		if      ((*sOpts=='i')||(*sOpts=='I')) iFlgs |= _FgNoCase_;
		else if ((*sOpts=='r')||(*sOpts=='R')) iFlgs |= _FgRight_;
		else if ((*sOpts=='e')||(*sOpts=='E')) iFlgs |= _FgDecimal_;
		else if ((*sOpts=='d')||(*sOpts=='D')) iFlgs |= _FgDescend_;
		else if ((*sOpts=='n')||(*sOpts=='N')) iFlgs |= _FgNoSort_;
		sOpts++;
	}
	iVld =1;
	iZkLe=strlen(sFind);			// lengte van de te zoeken string
	while (iStart<iEind)
	{
		iPos=iStart;
		while(iPos<iEind){
			if (sBuf[iPos]==cDlm) break;
			iPos++;
		}
		iRes=_strFlgCmp(sFind,iZkLe,sBuf+iStart,iPos-iStart,iFlgs);
		if (iRes==0){					// ok gevonden
			if (piVld) *piVld=iVld;
			return 1;
		}
		if (iRes<0) break;
		iVld++;
		iStart=++iPos;
	}
	if (piVld) *piVld=iVld;
	return 0;
}

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
int stlLocate(const char *sFind,STP pVA,int iD1,int iD2,int *piVld,const char *sOpts)
{
	int iStart=0,iEind,iRes;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind);
	_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind);
	if (iD2)      iRes=_strLocate(sFind,pVA->sBuf,iStart,iEind,_D3,piVld,sOpts);
	else if (iD1) iRes=_strLocate(sFind,pVA->sBuf,iStart,iEind,_D2,piVld,sOpts);
    else          iRes=_strLocate(sFind,pVA->sBuf,iStart,iEind,_D1,piVld,sOpts);
	_strTlUnlockVar(pVA);
	return iRes;
}
/* Locate a string in the data block
 *  sFind string to search for
 *  pVA   data to search in from
 *  cDlm  Char to use as delimiter
 *  piVld ptr to value where index is to be stored (optional)
 *  sOpts optional compare flags ("I"=case Insensitive, "N"=no Sorting, "R"=Right aligned "E"=Number aligned)
 * Return 1 when found (piVld is set to index)
 *        0 not found (piVld set to position to insert)
 */
int stlLocateDlm(const char *sFind,STP pVA,char cDlm,int *piVld,const char *sOpts)
{
	int iRes;
	if (pVA==NULL) return 0;
	_strTlLockVar(pVA);
	iRes=_strLocate(sFind,pVA->sBuf,0,pVA->iLen,cDlm,piVld,sOpts);
	_strTlUnlockVar(pVA);
	return iRes;
}
