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
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "stl_thread.h"
#include "stl_str.h"

int _strTlMaxBufLen=16000000;	/* Max alloc length */

#ifdef __linux__
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#endif

#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
//#  define _lxBreak_() __asm int 3
#  ifdef _DEBUG
#    include <crtdbg.h>
     static HANDLE hLogFile = NULL;
#  endif	// _DEBUG
#endif

#ifdef _DEBUG
#define _MaxStlSave_	20000
static STP _stlAlloc[_MaxStlSave_];
static int _stlInitL[_MaxStlSave_];	// length
static int _stlInitT[_MaxStlSave_];	// thread id
static int _stlAllocActive=0;
static int _stlAllocBreak=-1;
static void _stlAlocInit()
{
	static int iDone=0;
	if (iDone) return;
	memset(_stlAlloc,0,sizeof(_stlAlloc));
	iDone++;
}
static void _stlAllocAdd(STP sVal)
{
	int i;
	if (_stlAllocActive==0) return;
	_stlAlocInit();
	for (i=0;i<_MaxStlSave_;i++)
	{
		if (_stlAlloc[i]==NULL)
		{
			_stlAlloc[i]=sVal;
			_stlInitL[i]=sVal->iLen;
			_stlInitT[i]=stlGetThreadId();
			if (_stlAllocBreak == _stlInitL[i])
				_lxBreak_();
			return;
		}
	}
}
static void _stlAllocDel(STP sVal)
{
	int i;
	if (_stlAllocActive==0) return;
	_stlAlocInit();
	for (i=0;i<_MaxStlSave_;i++)
	{
		if (_stlAlloc[i]==sVal)
		{
			_stlAlloc[i]=NULL;
			return;
		}
	}
}
// Break on STL init alt specifiek length
void stlAllocBreak(int iLen)
{
	_stlAllocBreak=iLen;
}
// Start monitoring memory allocs
void stlAllocMon(void)
{
	_stlAllocActive=1;
#ifdef _WIN32
	if (hLogFile == NULL)
		hLogFile = CreateFile ("Memory Leaks.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// Send all reports to Memory Leaks.txt
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, hLogFile );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, hLogFile );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, hLogFile );
#endif
}
// Save all allocated memory to file
void stlAllocSave(const char *sFileName)
{
	FILE *f1;
	int i;
	_stlAlocInit();

#ifdef _WIN32
	_CrtDumpMemoryLeaks();
	if (hLogFile) FlushFileBuffers(hLogFile);
#endif

	f1=fopen(sFileName,"wb");
	if (f1==NULL) return;
	fprintf(f1,"Index,Length(Init,Current),data\r\n");
	for (i=0;i<_MaxStlSave_;i++)
	{
		if (_stlAlloc[i])
		{
			fprintf(f1,"pid=%d l=%d,%d pn=%s d=%s\r\n",_stlInitT[i],_stlInitL[i],_stlAlloc[i]->iLen,stlThreadPidName(_stlInitT[i]),_stlAlloc[i]->sBuf);
		}
	}
	fclose(f1);
}
#endif





/* INTERNAL */
/* Lock data variable */
void _strTlLockVar(STP pVA)
{
	if (pVA==NULL) _strSafeExit(_StTlErrNullVar_);
	if (pVA->_uid != _StrDataUid_) _strSafeExit(_StTlErrInvalVar_);
	stlMutexLock(&pVA->_mux);
	if (pVA->iLen > pVA->iLenLoc)
	{
		printf("Warning STP expanded not using stlInsDel change please\n");
		pVA->iLenLoc=pVA->iLen;
	}
}

/* INTERNAL */
/* UnLock data variable */
void _strTlUnlockVar(STP pVA)
{
	if (pVA==NULL) _strSafeExit(_StTlErrNullVar_);
	if (pVA->_uid != _StrDataUid_) _strSafeExit(_StTlErrInvalVar_);
	stlMutexUnlock(&pVA->_mux);
}

/* INTERNAL */
/* Initialize a data variable at given length */
STP _strTlInitVar(int iLen)
{
	STP pVA;
	int iLenA = iLen + 1;
	if (iLen <   0) iLen=0;
	if (iLenA < 128) iLenA = 128;	// altijd minimaal 128 bytes nemen
	if ((_strTlMaxBufLen>0)&&(iLen>_strTlMaxBufLen)) _strSafeExit(_StTlErrMaxMemUsed_);
	pVA=malloc(sizeof(struct str_data));
	if (pVA==NULL) _strSafeExit(_StTlErrOutOfMem_);
	memset(pVA,0,sizeof(struct str_data));
	pVA->sBuf=malloc(iLenA);
	if (pVA->sBuf==NULL) _strSafeExit(_StTlErrOutOfMem_);
	pVA->_uid=_StrDataUid_;	/* Tag var with uniq ID */
	pVA->sBuf[iLen]=0;		/* Place terminating zero */
	pVA->iLen=iLen;			/* Set length of var */
	pVA->iLenLoc=iLenA;		/* allocated length of var */
	stlMutexInit(&pVA->_mux);
#ifdef _DEBUG
	_stlAllocAdd(pVA);
#endif
	return pVA;
}

/* INTERNAL */
/* Insert or delete a part of the data */
int _strTlInsDel(STP pVA,int iPos,int iLen)
{
	int iSize;
	if (iLen==0) return 0;
	if (iPos< 0) {printf("Warning strTlInsDel called with negative (%d) offset\n",iPos); iPos=0;}
	if (iLen<0){	/* Need te delete characters */
		if (iPos>pVA->iLen) return 0;	/* no data to delete */
		if (iPos-iLen>pVA->iLen)		/* delete less than requested */
			iLen=iPos-pVA->iLen;		/* Calculate max data to delete */
		if (iLen>=0) return 0;
		iSize=pVA->iLen-iPos+iLen;
		if (iSize) memmove(pVA->sBuf+iPos,pVA->sBuf+iPos-iLen,iSize);
		pVA->iLen+=iLen;
		pVA->sBuf[pVA->iLen]=0;
		return iLen;
	}
	/* Need to insert characters */
	if ((_strTlMaxBufLen>0)&&(pVA->iLen+iLen>_strTlMaxBufLen)) _strSafeExit(_StTlErrMaxMemUsed_);
	// tested this setup and is only gives significant effects on stlAppend family functions
	if (pVA->iLen+iLen+1 > pVA->iLenLoc)	// not enough memory expand my memory
	{
		pVA->iLenLoc =pVA->iLen + iLen + 256;
		pVA->iLenLoc+=pVA->iLenLoc / 2;		// 25% extra memory for future expansions
		pVA->sBuf=realloc(pVA->sBuf,pVA->iLenLoc);
		if (pVA->sBuf==NULL) _strSafeExit(_StTlErrOutOfMem_);
	}
	if (iPos>pVA->iLen) iPos=pVA->iLen;		// start pos was after end of string
	iSize=pVA->iLen+1-iPos;
	pVA->iLen+=iLen;
	if (iSize>0) memmove(pVA->sBuf+iPos+iLen,pVA->sBuf+iPos,iSize);
	else pVA->sBuf[pVA->iLen]=0;
	return iLen;
}

/* INTERNAL */
// Sets start and end pos of search character (from begin or end)
int _strTlGetOfs(char *sBuf,char sSrc,int iCnt,int *pStart,int *pEind)
{
	register int iStart=*pStart,iEind=*pEind;
	if (iCnt==0) return 0;		// We want it all return immediately
	if (iCnt>0){
		sBuf+=iStart;			// Set buffer pointer to start from where to search
		while (iStart<iEind){
			iStart++;
			if (*sBuf++==sSrc){
				iCnt--;
				if (iCnt==1) *pStart=iStart;
				else if (iCnt==0){
					if (pEind) *pEind=iStart-1;
					return 0;
				}
			}
		}
		if (iCnt>1) *pStart=*pEind;
		return (iCnt-1);
	}
	// Search backwards (from the end of the data)
	sBuf+=iEind-1;					// Don't bother if pointer becomes before actual data
	if (iCnt==-1){					// We want only the data beyond the end
		*pStart=*pEind;
		if (iEind>iStart) return 1;	// Data is avail (so store needs extra delimiter)
		return 0;
	}
	while (iEind>iStart){
		iEind--;
		if (*sBuf--==sSrc){
			iCnt++;
			if (iCnt==-2) *pEind=iEind;
			else if (iCnt==-1){
				if (pEind) *pStart=iEind+1;
				return 0;
			}
		}
	}
	if (iCnt<-2) *pEind=*pStart;	// We wanted further back than te beginning.
	return (iCnt+2);				// Signal it back to caller
}

// Sets start and end pos of search character (from begin or end)
int _strTlGetOfsW(wchar_t *sBuf,wchar_t sSrc,int iCnt,int *pStart,int *pEind)
{
	register int iStart=*pStart,iEind=*pEind;
	if (iCnt==0) return 0;		// We want it all return immediately
	if (iCnt>0){
		sBuf+=iStart;			// Set buffer pointer to start from where to search
		while (iStart<iEind){
			iStart++;
			if (*sBuf++==sSrc){
				iCnt--;
				if (iCnt==1) *pStart=iStart;
				else if (iCnt==0){
					if (pEind) *pEind=iStart-1;
					return 0;
				}
			}
		}
		if (iCnt>1) *pStart=*pEind;
		return (iCnt-1);
	}
	// Search backwards (from the end of the data)
	sBuf+=iEind-1;					// Don't bother if pointer becomes before actual data
	if (iCnt==-1){					// We want only the data beyond the end
		*pStart=*pEind;
		if (iEind>iStart) return 1;	// Data is avail (so store needs extra delimiter)
		return 0;
	}
	while (iEind>iStart){
		iEind--;
		if (*sBuf--==sSrc){
			iCnt++;
			if (iCnt==-2) *pEind=iEind;
			else if (iCnt==-1){
				if (pEind) *pStart=iEind+1;
				return 0;
			}
		}
	}
	if (iCnt<-2) *pEind=*pStart;	// We wanted further back than te beginning.
	return (iCnt+2);				// Signal it back to caller
}



//**************************************************************
//**
//** P U B L I C   F U N C T I O N S
//**

/* these 2 function are not thread save.
** eg if 1 thread has a pointer an other thread can use more than 15 pointers
** rendering the pointer of thread 1 into nowhere (use pointers per thread ?)
** this can be very slow en memory consuming.
const char *stlStringConsume(STP sValue)
{
	static volatile int iIdx = 0;
	static STP sAR[15] = {0};
	int i = iIdx = ((iIdx + 1) % 15);
	if (sAR[i]) stlFree(sAR[i]);
	sAR[i] = sValue;
	if (sAR[i] == NULL) return NULL;
	return sAR[i]->sBuf;
}
const char *stlStringConsumeNN(STP sValue)
{
	const char *res = stlStringConsume(sValue);
	if (res) return res;
	return "";
}
*/

/* Release memory used by toolbox string */
void stlFree(STP pVA)
{
	if (pVA==NULL) return;
#ifdef _DEBUG
	_stlAllocDel(pVA);
#endif
	_strTlLockVar(pVA);
	_strTlUnlockVar(pVA);
	if (pVA->sBuf) free(pVA->sBuf);
	stlMutexRelease(&pVA->_mux);
	memset(pVA,0,sizeof(struct str_data));
	free(pVA);
}

/* Create new STP data with iBytes length (excluding terminating 0)
 *	iBytes  numb of bytes space to alloc
 *  cFill   char to fill data with
 * Return STP data with 
 */
STP stlInitLen(int iBytes,char cFill)
{
	STP sRes=_strTlInitVar(iBytes);
	if (sRes) memset(sRes->sBuf,cFill,sRes->iLen);
	return sRes;
}
/* Create new STP data with iBytes length (excluding terminating 0)
 *	iBytes  numb of bytes space to alloc
 *  cFill   char to fill data with
 * Return STP data with 
 */
STP stlInitLenSrc(int iBytes,char *src)
{
	STP sRes=_strTlInitVar(iBytes);
	if (sRes)
	{
		memcpy(sRes->sBuf,src,sRes->iLen);
		sRes->sBuf[sRes->iLen] = 0;
	}
	return sRes;
}

/* Initialize new struct ptr with string
 */
STP stlSetSt(const char *pStr)
{
	int iLen;
	STP pVA;
	if (pStr==NULL) pStr="";
	iLen=(int)strlen(pStr);
	pVA=_strTlInitVar(iLen);
	if (iLen>0) memcpy(pVA->sBuf,pStr,iLen);
	return pVA;
}

/* Initialize new strict withe the same contents
 */
STP stlCopy(STP pVA)
{
	STP sRes;
	if (pVA==NULL) return NULL;
	_strTlLockVar(pVA);
	sRes=_strTlInitVar(pVA->iLen);
	memcpy(sRes->sBuf,pVA->sBuf,pVA->iLen);
	_strTlUnlockVar(pVA);
	return sRes;
}
/* Exchange the content of two data blocks
 *	Useful if u want to replace all data in a block as function var.
 */
void stlExchange(STP sV1,STP sV2)
{
	char *sBuf;
	int   iLen,iLea;
	if ((sV1==NULL)||(sV2==NULL)) return;
	_strTlLockVar(sV1);
	_strTlLockVar(sV2);
	sBuf=sV1->sBuf;
	iLen=sV1->iLen;
	iLea=sV1->iLenLoc;
	sV1->sBuf   =sV2->sBuf;
	sV1->iLen   =sV2->iLen;
	sV1->iLenLoc=sV2->iLenLoc;
	sV2->sBuf   =sBuf;
	sV2->iLen   =iLen;
	sV2->iLenLoc=iLea;
	_strTlUnlockVar(sV1);
	_strTlUnlockVar(sV2);
}
/* Rotate STL data 
*/
void stlRotate(STP sData)
{
	STP sRes,sRow,sTmp,*sAr;
	int i,nri,j,nrj=0;
	nri=stlCount(sData,_D1)+1;
	sAr=malloc(nri*sizeof(STP));
	if (sAr==NULL) return;
	for (i=0;i<nri;i++){
		sAr[i]=stlGetStr(sData,i+1,0,0);
		j=stlCount(sAr[i],_D2)+1;
		if (j>nrj) nrj=j;
	}
	sRes=stlSetSt("");
	for (j=0;j<nrj;j++)
	{
		if (j) sRow=stlSetSt(_sD1); else sRow=stlSetSt("");
		for (i=0;i<nri;i++)
		{
			sTmp=stlGetDlm(sAr[i],j+1,_D2);
			stlAppendStp(sRow,sTmp);
			stlAppendCh (sRow,_D2);
			stlFree(sTmp);
		}
		// Remove needles data
		while (sRow->iLen){if (sRow->sBuf[sRow->iLen-1]==_D2) stlInsDel(sRow,sRow->iLen-1,-1); else break; }
		stlAppendStp(sRes,sRow);
		stlFree(sRow);
	}
	stlExchange(sData,sRes);				// exchange original with rotated data
	for (i=0;i<nri;i++) stlFree(sAr[i]);	// release temp memory
	stlFree(sRes);							// release original data
	free(sAr);								// release array
}


/* Count number of occurrences of selected char in buffer
 *	sBuf buffer to string to count in
 *  ch   char to be counted
 * return number of occurrences.
 */
int stlCountChr(const char *sBuf,char ch)
{
	int iCnt=0;
	if (sBuf==NULL) return 0;
	while(1){
		if (*sBuf==ch) iCnt++;
		if (*sBuf==0 ) break;
		sBuf++;
	}
	return iCnt;
}

/* Count number of occurrences of selected char in buffer
 *	sBuf STP data to count in
 *  ch   char to be counted
 * return number of occurrences.
 */
int stlCount(STP sV1,char ch)
{
	int i,iCnt=0;
	if (sV1==NULL) return 0;
	_strTlLockVar(sV1);
	for (i=0;i<sV1->iLen;i++){
		if (sV1->sBuf[i]==ch) iCnt++;
	}
	_strTlUnlockVar(sV1);
	return iCnt;
}

/* Get index of nTh (1=first, 2 second, -1=last) occurrence of cDlm in the buffer
** Return -1 not found
**        -2 NULL data ptr
*/
int stlIndexOfN(STP sV1,char cDlm,int iD1)
{
	int iStart=0,iEind,iRes;
	if (sV1==NULL) return -2;
	if (iD1 < 0) iD1 -= 2;		// so -1 is last occurrence of cDlm
	_strTlLockVar(sV1);
	iEind=sV1->iLen;
	iRes = _strTlGetOfs(sV1->sBuf,cDlm,iD1,&iStart,&iEind);
	if ((iRes == 0) && (iEind == sV1->iLen)) iEind = -1;
	_strTlUnlockVar(sV1);
	if (iRes == 0) return iEind;
	return -1;
}

/* Get index of first occurrence of cDlm in the buffer
** Return -1 not found
**        -2 NULL data ptr
*/
int stlIndexOf(STP sV1,char cDlm)
{
	return stlIndexOfN(sV1,cDlm,1);
}



/* Insert or delete a number of characters at a specifiek position
 * This function is safe against mistakes
 *  pVA   STP data to delete or insert characters
 *  iPos  Position in dat to start at
 *  iLen  Number of bytes to delete (<0) or insert (>0)
 * Return New length of STP data
 */
int stlInsDel(STP pVA,int iPos,int iLen)
{
	int iRes=0;
	if (pVA!=NULL){
		_strTlLockVar(pVA);
		iRes=_strTlInsDel(pVA,iPos,iLen);
		_strTlUnlockVar(pVA);
	}
	return iRes;
}

