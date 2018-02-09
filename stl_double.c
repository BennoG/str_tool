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
#include <math.h>

#include "stl_str.h"
#include "internal.h"


double stlGetDlmDouble(STP pVA,int iD1,char cDlm)
{
	double dRes;
	STP sVA;
	sVA=stlGetDlm(pVA,iD1,cDlm);
	dRes=atof(sVA->sBuf);
	stlFree(sVA);
	return dRes;
}

double stlGetDouble(STP pVA,int iD1,int iD2,int iD3)
{
	double dRes;
	STP sVA;
	if (pVA==NULL) return 0;
	sVA=stlGetFld(pVA,iD1,iD2,iD3);
	dRes=atof(sVA->sBuf);
	stlFree(sVA);
	return dRes;
}

static void _stlDouble2str(double dValue,char *sBuf,int iBufLen)
{
	int    le;
	double dAbsVal=fabs(dValue);
	iBufLen -= 5;
	if (dAbsVal<1) le=8; else le=(int)(log10(dAbsVal)+1);
	if (le<8) le=8;
	if (le>iBufLen) le=iBufLen;
	if (gcvt(dValue,le,sBuf)==NULL)
		printf("Error _stlDouble2str");// op linux moest wat met ruterun waarde gebeuren
	le=strlen(sBuf);
	if ((le>0) && (sBuf[le-1]=='.')) sBuf[le-1]=0;
}


void stlStoreDlmDouble(double dVal,STP pVA,int iD1,char cDlm)
{
	char sBuf[65];
	if (pVA==NULL) return;
	_stlDouble2str(dVal,sBuf,sizeof(sBuf));
	stlStoreDlm(sBuf,pVA,iD1,cDlm);
}
void stlStoreDouble(double dVal,STP pVA,int iD1,int iD2,int iD3)
{
	char sBuf[65];
	if (pVA==NULL) return;
	_stlDouble2str(dVal,sBuf,sizeof(sBuf));
	stlStoreStr(sBuf,pVA,iD1,iD2,iD3);
}
void stlInsertDouble(double dVal  ,STP pVA,int iD1,int iD2,int iD3)
{
	char sBuf[65];
	if (pVA==NULL) return;
	_stlDouble2str(dVal,sBuf,sizeof(sBuf));
	stlInsertStr(sBuf,pVA,iD1,iD2,iD3);
}
