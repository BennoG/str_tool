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
#include <stdio.h>
#include <string.h>

#include "stl_str.h"
#include "stl_tcp.h"
#include "stl_thread.h"
#include "internal.h"

#if defined(_DEBUG) || defined(__linux__)
// Internal testing of functionality
static int testStrTlGetOfs(void)
{
	int iStart,iEind,iRes;
	char *st="123" _sD1 _sD1 "456";
	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,0,&iStart,&iEind);	// 0-8 (0)
	if ((iRes!=0)||(iStart!=0)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 1\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,1,&iStart,&iEind);	// 0-3 (0)
	if ((iRes!=0)||(iStart!=0)||(iEind!=3)){printf("==FATAL== testStrGetOfs test 2\n"); return 1;}
	
	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,2,&iStart,&iEind);	// 4-4 (0)
	if ((iRes!=0)||(iStart!=4)||(iEind!=4)){printf("==FATAL== testStrGetOfs test 3\n"); return 1;}
	
	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,3,&iStart,&iEind);	// 5-8 (0)
	if ((iRes!=0)||(iStart!=5)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 4\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,4,&iStart,&iEind);	// 8-8 (1)
	if ((iRes!=1)||(iStart!=8)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 5\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,5,&iStart,&iEind);	// 8-8 (2)
	if ((iRes!=2)||(iStart!=8)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 6\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,-1,&iStart,&iEind);	// 8-8 (1)
	if ((iRes!=1)||(iStart!=8)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 7\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,-2,&iStart,&iEind);	// 5-8 (0)
	if ((iRes!=0)||(iStart!=5)||(iEind!=8)){printf("==FATAL== testStrGetOfs test 8\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,-3,&iStart,&iEind);	// 4-4 (0)
	if ((iRes!=0)||(iStart!=4)||(iEind!=4)){printf("==FATAL== testStrGetOfs test 9\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,-4,&iStart,&iEind);	// 0-3 (0)
	if ((iRes!=0)||(iStart!=0)||(iEind!=3)){printf("==FATAL== testStrGetOfs test 10\n"); return 1;}

	iStart=0;iEind=strlen(st);
	iRes=_strTlGetOfs(st,_D1,-5,&iStart,&iEind);	// 0-0 (-1)
	if ((iRes!=-1)||(iStart!=0)||(iEind!=0)){printf("==FATAL== testStrGetOfs test 11\n"); return 1;}
	
	return 0;
}


// Internal testing of functionality
static int testStrFld(void)
{
	STP pVA,pRes;
	pVA=stlSetSt("123" _sD1 _sD1 "456" _sD2 "789" _sD3 "000");
	
	pRes=stlGetFld(pVA,0,0,0);
	if (strcmp(pRes->sBuf,pVA->sBuf)){printf("==FATAL== testStrFld test 1\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,1,0,0);
	if (strcmp(pRes->sBuf,"123")){printf("==FATAL== testStrFld test 2\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,2,0,0);
	if (strcmp(pRes->sBuf,"")){printf("==FATAL== testStrFld test 3\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,0,0);
	if (strcmp(pRes->sBuf,"456" _sD2 "789" _sD3 "000")){printf("==FATAL== testStrFld test 4\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,4,0,0);
	if (strcmp(pRes->sBuf,"")){printf("==FATAL== testStrFld test 5\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,1,0);
	if (strcmp(pRes->sBuf,"456")){printf("==FATAL== testStrFld test 6\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,2,0);
	if (strcmp(pRes->sBuf,"789" _sD3 "000")){printf("==FATAL== testStrFld test 7\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,3,0);
	if (strcmp(pRes->sBuf,"")){printf("==FATAL== testStrFld test 8\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,2,1);
	if (strcmp(pRes->sBuf,"789")){printf("==FATAL== testStrFld test 9\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,2,2);
	if (strcmp(pRes->sBuf,"000")){printf("==FATAL== testStrFld test 10\n"); return 1;}
	stlFree(pRes);

	pRes=stlGetFld(pVA,3,2,3);
	if (strcmp(pRes->sBuf,"")){printf("==FATAL== testStrFld test 11\n"); return 1;}
	stlFree(pRes);
	// From here the backward serarches

	return 0;
}

static int testStlStoreStr(void)
{
	STP pVA;
	pVA=stlSetSt("");
	
	stlStoreStr("abc",pVA,1,0,0);
	if (strcmp(pVA->sBuf,"abc")){printf("==FATAL== testStlStoreFld test 1\n"); return 1;}

	stlStoreStr("123",pVA,3,0,0);
	if (strcmp(pVA->sBuf,"abc" _sD1 _sD1 "123")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}

	stlStoreStr("A",pVA,2,-1,0);
	if (strcmp(pVA->sBuf,"abc" _sD1 "A" _sD1 "123")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}
	stlStoreStr("B",pVA,2,-1,0);
	stlStoreStr("C",pVA,2,-1,0);
	if (strcmp(pVA->sBuf,"abc" _sD1 "A" _sD2 "B" _sD2 "C" _sD1 "123")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}
	stlFree(pVA);

	pVA=stlSetSt("");
	stlStoreStr("A",pVA,3,0,0);
	if (strcmp(pVA->sBuf,_sD1 _sD1 "A")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}
	stlFree(pVA);

	pVA=stlSetSt("");
	stlStoreStr("A",pVA,4,3,2);
	if (strcmp(pVA->sBuf,_sD1 _sD1 _sD1 _sD2 _sD2 _sD3 "A")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}

	stlStoreStr("B",pVA,4,3,-3);
	if (strcmp(pVA->sBuf,_sD1 _sD1 _sD1 _sD2 _sD2 "B" _sD3 "A")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}

	stlStoreStr("C",pVA,4,3,-4);
	if (strcmp(pVA->sBuf,_sD1 _sD1 _sD1 _sD2 _sD2 "C" _sD3 "B" _sD3 "A")){printf("==FATAL== testStlStoreFld test 3\n"); return 1;}
	stlFree(pVA);

	return 0;
}

void mytask(void *p)
{
	static struct stlMutex mux=stlMutexInitializer;
	static int iCnt=0,iFlg=0;
	int i;
	printf("thread enter %s\n",(char*)p);
	for (i=1;i<500;i++){
		stlMutexLock(&mux);
		if (iFlg) printf("Error on lock %d\n",iCnt);
		iFlg=1;
		iCnt++;
		stlMsWait(1);
		iFlg=0;
		stlMutexUnlock(&mux);
	}
	printf("Cnt=%d\n",iCnt);
}

// TODO:
// TCP basis Win32
// TCP basis Linux
// TCP (thread server)

// SQL (odbc)

static void cpuTimeTest()
{
	int i;
	unsigned int ims = stlMsTimer(0);
	unsigned int icpu = stlCpuTimer(0);
	for (i = 0; i < 10; i++)
	{
		stlMsWait(1000);
		printf("ms %5d cpu %7d",stlMsTimer(ims),stlCpuTimer(icpu));
	}
}

int main(void)
{
	int iRes,iTim;
	struct stlTcpConn *stc;
	STP sLin;
	
	cpuTimeTest();

	iTim=stlMsTimer(0);
	iRes=stlTcpConnect("192.168.90.5",5000);
	iTim=stlMsTimer(iTim);
	printf("socket res=%d time=%d ms\n",iRes,iTim);

	iTim=stlMsTimer(0);
	iRes=stlTcpConnect("213.84.120.47",10000);
	iTim=stlMsTimer(iTim);
	printf("socket res=%d time=%d ms\n",iRes,iTim);

	iTim=stlMsTimer(0);
	stc=stlTcpInit(iRes);
	stlTcpWrite(stc,(unsigned char*)"list\r\n",6,10000);
	while (1){
		sLin=stlTcpReadLine(stc,1000);
		if (sLin==NULL) break;
		printf("Res=%d %s\n",stlMsTimer(iTim),sLin->sBuf);
		stlFree(sLin);
	}
	stlTcpRelease(stc);
	return 0;
}


int main_old()
{
	unsigned int uTm;
	int iRes=0;
	uTm=stlUsTimer(0);
	iRes+=testStrTlGetOfs();
	iRes+=testStrFld();
	iRes+=testStlStoreStr();
	uTm=stlUsTimer(uTm);
	printf("Test took %d us\n",uTm);

//	uTm=stlUsTimer(0);
//	for (iRes=0;iRes<10000000;iRes++) stlUsTimer(0);
//	uTm=stlUsTimer(uTm);printf("Test took %d us\n",uTm);

	stlThreadStart(mytask,0,"test1","test1");
	stlThreadStart(mytask,0,"test2","test2");
	stlSecWait(20);
	return 0;
}
#endif
