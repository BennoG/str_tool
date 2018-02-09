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

/* Store value into delimited data
*  pStr String to store in data
*  pVA  STP struct to store data in
*  iD1  index1  (neg values count from the back)
*  cDlm char to use as delimiter
*/
void stlStoreDlm(const char *pStr,STP pVA,int iD1,char cDlm)
{
	char *p;
	int iOldLen,iNewLen,iLen;
	int iStart=0,iEind;
	if (pVA==NULL) return;
	if (pStr==NULL) pStr="";
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	iD1=_strTlGetOfs(pVA->sBuf,cDlm,iD1,&iStart,&iEind);

	iLen=strlen(pStr);
	iOldLen=iEind-iStart;
	iNewLen=iLen+abs(iD1);
	_strTlInsDel(pVA,iStart,iNewLen-iOldLen);
	p=pVA->sBuf+iStart;
	while (iD1>0){*p++=cDlm;iD1--;}
	if (iLen){memcpy(p,pStr,iLen); p+=iLen;}
	while (iD1<0){*p++=cDlm;iD1++;}
	_strTlUnlockVar(pVA);
}


/* Store value into delimited data
 *  pStr String to store in data
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlStoreStr(const char *pStr,STP pVA,int iD1,int iD2,int iD3)
{
	char *p;
	int iOldLen,iNewLen,iLen;
	int iStart=0,iEind;
	if (pVA==NULL) return;
	if (pStr==NULL) pStr="";
	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	iD1=_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind);
	iD2=_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind);
	iD3=_strTlGetOfs(pVA->sBuf,_D3,iD3,&iStart,&iEind);

	iLen=strlen(pStr);
	iOldLen=iEind-iStart;
	iNewLen=iLen+abs(iD1)+abs(iD2)+abs(iD3);
	_strTlInsDel(pVA,iStart,iNewLen-iOldLen);
	p=pVA->sBuf+iStart;
	while (iD1>0){*p++=_D1;iD1--;}
	while (iD2>0){*p++=_D2;iD2--;}
	while (iD3>0){*p++=_D3;iD3--;}
	if (iLen){memcpy(p,pStr,iLen); p+=iLen;}
	while (iD3<0){*p++=_D3;iD3++;}
	while (iD2<0){*p++=_D2;iD2++;}
	while (iD1<0){*p++=_D1;iD1++;}
	_strTlUnlockVar(pVA);
}

void stlInsertDlm(const char *pStr,STP pVA,int iD1,char cDlm)
{
	char *p;
	int iNewLen,iLen;
	int iStart=0,iEind;
	if (pVA==NULL) return;
	if (pStr==NULL) pStr="";

	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	iD1=_strTlGetOfs(pVA->sBuf,cDlm,iD1,&iStart,&iEind);

	iLen=strlen(pStr);
	iNewLen=iLen+abs(iD1)+1;

	_strTlInsDel(pVA,iStart,iNewLen);
	p=pVA->sBuf+iStart;
	while (iD1>0){*p++=cDlm;iD1--;}
	if (iLen){memcpy(p,pStr,iLen); p+=iLen;}
	*p++=cDlm;
	while (iD1<0){*p++=cDlm;iD1++;}
	_strTlUnlockVar(pVA);
}

/* Insert value into delimited data
 *  pStr String to insert in data
 *  pVA  value to get data from
 *  iD1  index1  (neg values count from the back)
 *  iD2  index2  (neg values count from the back)
 *  iD3  index3  (neg values count from the back)
 */
void stlInsertStr(const char *pStr,STP pVA,int iD1,int iD2,int iD3)
{
	char *p,cD;
	int iNewLen,iLen;
	int iStart=0,iEind;
	if (pVA==NULL) return;
	if (pStr==NULL) pStr="";
	if (iD3){							// Decide witch char to use as delimiter
		cD=_D3;
	}else{
		if (iD2) cD=_D2;
		else     cD=_D1;
	}

	_strTlLockVar(pVA);
	iEind=pVA->iLen;
	iD1=_strTlGetOfs(pVA->sBuf,_D1,iD1,&iStart,&iEind);
	iD2=_strTlGetOfs(pVA->sBuf,_D2,iD2,&iStart,&iEind);
	iD3=_strTlGetOfs(pVA->sBuf,_D3,iD3,&iStart,&iEind);

	iLen=strlen(pStr);
	iNewLen=iLen+abs(iD1)+abs(iD2)+abs(iD3)+1;
	
	_strTlInsDel(pVA,iStart,iNewLen);
	p=pVA->sBuf+iStart;
	while (iD1>0){*p++=_D1;iD1--;}
	while (iD2>0){*p++=_D2;iD2--;}
	while (iD3>0){*p++=_D3;iD3--;}
	if (iLen){memcpy(p,pStr,iLen); p+=iLen;}
	*p++=cD;
	while (iD3<0){*p++=_D3;iD3++;}
	while (iD2<0){*p++=_D2;iD2++;}
	while (iD1<0){*p++=_D1;iD1++;}
	_strTlUnlockVar(pVA);
}
/* Copy 1 field from source data into another field in destination data
 *	sDst  destination data
 *  iD1,iD2,iD3 Index numbers destination (neg values count from the back)
 *	sSrc  source data
 *  iS1,iS2,iS3 Index numbers source (neg values count from the back)
 */
void stlCpyVld(STP sDst,int iD1,int iD2,int iD3,STP sSrc,int iS1,int iS2,int iS3)
{
	STP sTmp;
	sTmp=stlGetStr(sSrc,iS1,iS2,iS3);
	if ((iD3)&&(iS3==0)){
		if (iS2) stlConvert(sTmp,_D3,(char)252);
		if (iS1) stlConvert(sTmp,_D2,(char)252);
	}
	if ((iD2)&&(iS2==0)){
		if (iS1) stlConvert(sTmp,_D2,_D3);
	}
	if ((iD1)&&(iS1==0)){
		stlConvert(sTmp,_D2,_D3);
		stlConvert(sTmp,_D1,_D2);
	}
	stlStoreStr(sTmp->sBuf,sDst,iD1,iD2,iD3);
	stlFree(sTmp);
}
