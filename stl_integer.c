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


int stlGetDlmInt(STP pVA,int iD1,char cDlm)
{
	int iRes;
	STP sVA;
	sVA=stlGetDlm(pVA,iD1,cDlm);
	iRes=atoi(sVA->sBuf);
	stlFree(sVA);
	return iRes;
}

int stlGetDlmIntAuto(STP pVA,int iD1,char cDlm)
{
	int iRes;
	STP sVA;
	sVA=stlGetDlm(pVA,iD1,cDlm);
	iRes=strtol(sVA->sBuf,NULL,0);
	stlFree(sVA);
	return iRes;
}
int stlGetDlmIntAutoD(STP pVA,int iD1,char cDlm,int iDefault)
{
	STP sVA;
	sVA=stlGetDlm(pVA,iD1,cDlm);
	if ((sVA)&&(sVA->iLen)) iDefault = strtol(sVA->sBuf,NULL,0);
	stlFree(sVA);
	return iDefault;
}

int stlGetDlmHex(STP pVA,int iD1,char cDlm)
{
	int iRes;
	STP sVA;
	sVA=stlGetDlm(pVA,iD1,cDlm);
	iRes=strtoul(sVA->sBuf,NULL,16);
	stlFree(sVA);
	return iRes;
}


int stlGetInt(STP pVA,int iD1,int iD2,int iD3)
{
	int iRes;
	STP sVA;
	if (pVA==NULL) return 0;
	sVA=stlGetFld(pVA,iD1,iD2,iD3);
	iRes=atoi(sVA->sBuf);
	stlFree(sVA);
	return iRes;
}
unsigned int stlGetHex(STP pVA,int iD1,int iD2,int iD3)
{
	int iRes;
	STP sVA;
	if (pVA==NULL) return 0;
	sVA=stlGetFld(pVA,iD1,iD2,iD3);
	iRes=strtoul(sVA->sBuf,NULL,16);
	stlFree(sVA);
	return iRes;
}
/* binair 2 tallig stelsel
*/
unsigned int stlGetBin(STP pVA,int iD1,int iD2,int iD3)
{
	char *pVal;
	int iRes;
	STP sVA;
	if (pVA==NULL) return 0;
	sVA=stlGetFld(pVA,iD1,iD2,iD3);
	iRes=strtoul(sVA->sBuf,&pVal,2);
	stlFree(sVA);
	return iRes;
}

void stlStoreDlmInt(int iVal,STP pVA,int iD1,char cDlm)
{
	char buf[35];
	if (pVA==NULL) return;
	itoa(iVal,buf,10);
	stlStoreDlm(buf,pVA,iD1,cDlm);
}
void stlStoreInt(int iVal,STP pVA,int iD1,int iD2,int iD3)
{
	char buf[35];
	if (pVA==NULL) return;
	itoa(iVal,buf,10);
	stlStoreStr(buf,pVA,iD1,iD2,iD3);
}
void stlStoreHex(int iVal,STP pVA,int iD1,int iD2,int iD3)
{
	char buf[35];
	if (pVA==NULL) return;
	itoa(iVal,buf,16);
	stlStoreStr(buf,pVA,iD1,iD2,iD3);
}
void stlStoreBin(int iVal,STP pVA,int iD1,int iD2,int iD3)
{
	char buf[35];
	if (pVA==NULL) return;
	itoa(iVal,buf,2);
	stlStoreStr(buf,pVA,iD1,iD2,iD3);
}

void stlInsertInt(int iVal  ,STP pVA,int iD1,int iD2,int iD3)
{
	char buf[35];
	if (pVA==NULL) return;
	itoa(iVal,buf,10);
	stlInsertStr(buf,pVA,iD1,iD2,iD3);
}
