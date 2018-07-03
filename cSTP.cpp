/* ---------------------------------------------------------------------*
*                    Advanced Network Services v.o.f.,                  *
*              Copyright (c) 2002-2016 All Rights reserved              *
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
#include "stl_str.h"
#include "internal.h"

using namespace ansStl;

bool ansStlDebug = false;

#ifdef _WIN32
#  include <shlwapi.h>
#  ifdef _WIN64
#    define int64_t int
#  else
#    define int64_t int
#  endif
#  ifndef va_copy
#     define va_copy(a,b) (a=b)
#  endif // va_copy
#endif

#ifdef __linux__
#  ifdef __aarch64__
#  else
#    ifdef __x86_64__
#    else
#      define int64_t int		// 32 bit Linux
#    endif
#  endif
#  ifdef __GLIBC_PREREQ
#  else
#    define va_copy(a,b) (a=b)
#  endif
#endif

#if defined(__linux__) || defined(__APPLE__)
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#endif
#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
//#  define _lxBreak_() __asm int 3
#endif


cST::cST()
{
	initVars();
	if (ansStlDebug) printf("new cST 1 (0x%X)",(int)(int64_t)this);
}
cST::~cST()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (sBuf ){ delete sBuf;  sBuf  = NULL; }
	if (sWBuf){ delete sWBuf; sWBuf = NULL; }
	_uid = 0;
	iLen = iLenLoc = 0;
	if (ansStlDebug) printf("delete cST (0x%X)",(int)(int64_t)this);
}

cST::cST(STP p,bool consume /* = false */)
{
	initVars();
	if (ansStlDebug) printf("new cST 2 (0x%X)",(int)(int64_t)this);
	set(p,consume);
}
cST::cST(int len,const char *p)
{
	initVars();
	if (ansStlDebug) printf("new cST 3 (0x%X)",(int)(int64_t)this);
	set(len,p);
}
cST::cST(int len,const wchar_t *p)
{
	initVars();
	if (ansStlDebug) printf("new cST 4 (0x%X)",(int)(int64_t)this);
	set(len,p);
}
cST::cST(const wchar_t *p,...)
{
	initVars();
	if (ansStlDebug) printf("new cST 5 (0x%X)",(int)(int64_t)this);
	va_list ap;
	va_start(ap,p);
	setA(ap,p);
	va_end(ap);
}
cST::cST(const char *p,...)
{
	initVars();
	if (ansStlDebug) printf("new cST 6 (0x%X)",(int)(int64_t)this);
	va_list ap;
	va_start(ap,p);
	setA(ap,p);
	va_end(ap);
}
cST::cST(va_list ar,const char *p)
{
	initVars();
	if (ansStlDebug) printf("new cST 7 (0x%X)",(int)(int64_t)this);
	setA(ar,p);
}
cST::cST(va_list ar,const wchar_t *p)
{
	initVars();
	if (ansStlDebug) printf("new cST 8 (0x%X)",(int)(int64_t)this);
	setA(ar,p);
}
cST::cST(const cST &strSrc)
{
	initVars();
	if (ansStlDebug) printf("new cST 9 (0x%X)",(int)(int64_t)this);
	set(&strSrc);
}
cST::cST(const std::string &src)
{
	initVars();
	if (ansStlDebug) printf("new cST 10 (0x%X)",(int)(int64_t)this);
	set(src);
}

cST::operator const char *()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return this->buf();
}

cST::operator const char *() const
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return ((cST*)this)->buf();
}
cST::operator const unsigned char *()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return (const unsigned char *)this->buf();
}

cST::operator const unsigned char *() const
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return (const unsigned char *)((cST*)this)->buf();
}

cST& cST::operator=(const char* src)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// Copy the right hand side to the buffer.
	this->set(0,src);
	return *this;
}

cST& cST::operator=(const cST &strSrc)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// If the right hand side is not the left hand side...
	if(this != &strSrc) {
		// Copy the right hand side to the buffer.
		if (strSrc.sBuf != NULL)
			set(strSrc.iLen,strSrc.sBuf);
		else
			set(strSrc.iLen,strSrc.sWBuf);
		isUTF8 = strSrc.isUTF8;
	}
	return *this;
}
cST& cST::operator = (STP sVal)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	this->set(sVal);
	return *this;
}


cST cST::operator +(const char *src)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	cST n(buf());
	if (src) n.append(strlen(src),src);
	return n;
}

cST& cST::operator+=(const char* src)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// Copy the right hand side to the buffer.
	if (src) this->append(strlen(src),src);
	return *this;
}

cST& cST::operator+=(const char src)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// Copy the right hand side to the buffer.
	this->append(src);
	return *this;
}

cST& cST::operator+=(const cST &strSrc)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// Copy the right hand side to the buffer.
	if (((cST&)strSrc).is8bit())
		this->append(strSrc.iLen,strSrc.sBuf);
	else
		this->append(strSrc.iLen,strSrc.sWBuf);
	return *this;
}

char& cST::operator [](int iIdx)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (sBuf == NULL) update8bit();
	if (iIdx < 0) iIdx = iLen + iIdx;	// zodat -1 laatste teken word -2 op 1 na latste
	if ((iIdx<0)||(iIdx>=iLen)) iIdx = 0;
	return sBuf[iIdx];
}

void cST::set(STP p,bool consume /* = false */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p) set(p->iLen,p->sBuf);
	else   set(0,"");
	if (consume) stlFree(p);
}
void cST::set(int len,const wchar_t *p)
{
 	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((len <= 0) && (p != NULL)) len=wcslen((wchar_t*)p);
	iLen = len;
	if (iLen < 0) iLen = 0;
	if ((_strTlMaxBufLen>0)&&(iLen > _strTlMaxBufLen)) throw "maximum alloc size reached";
	if (iLen + 5 > iLenLoc)
	{
		iLenLoc = ((iLen + 50) | 0xFF) + 1;
		//	iLenLoc = iLen + 250;
		if (sWBuf){delete sWBuf; sWBuf = NULL;}
	}
	if (sBuf ){delete sBuf;  sBuf  = NULL;}
	if (sWBuf == NULL) sWBuf = new wchar_t[iLenLoc];
	if (p) memcpy(sWBuf, p, iLen    * sizeof(wchar_t));
	else   memset(sWBuf, 0, iLenLoc * sizeof(wchar_t));
	sWBuf[iLen] = 0;
}

static bool testUtf8(const char *buf,int iLen)
{
	if ((buf == NULL) || (iLen < 3)) return false;
	const unsigned char *ub = (const unsigned char *)(buf);
	if (iLen < 3) return false;
	if (ub == NULL) return false;
	if (ub[0] != 0xEF) return false;
	if (ub[1] != 0xBB) return false;
	if (ub[2] != 0xBF) return false;
	return true;
}

void cST::set(int len,const char *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((len <= 0) && (p != NULL)) len=strlen(p);
	iLen = len;
	if (iLen < 0) iLen = 0;
	if ((_strTlMaxBufLen>0)&&(iLen > _strTlMaxBufLen)) throw "maximum alloc size reached";

	if (testUtf8(p,iLen)){ isUTF8 = true; p+=3; iLen -= 3; }	// 0xEF 0xBB 0xBF (utf-8 start)

	if (iLen + 5 > iLenLoc)
	{
		iLenLoc = ((iLen + 50) | 0xFF) + 1;
		//	iLenLoc = iLen + 250;
		if (sBuf){delete sBuf; sBuf = NULL;}
	}
	if (sWBuf){delete sWBuf; sWBuf = NULL;}
	if (sBuf == NULL) sBuf = new char[iLenLoc];
	if (p) memcpy(sBuf, p, iLen    * sizeof(char));
	else   memset(sBuf, 0, iLenLoc * sizeof(char));
	sBuf[iLen] = 0;
}
void cST::set(const cST *val)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (val == NULL) return;
	if (val->sBuf)
		set(val->iLen,val->sBuf);
	else if (val->sWBuf)
		set(val->iLen,val->sWBuf);
	isUTF8 = val->isUTF8;
}
void cST::set(const std::string &val)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	set(val.length(),val.c_str());
}
void cST::set(const char *val)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	set(0,val);
}
void cST::set(const wchar_t *val)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	set(0,val);
}

void cST::setf(const wchar_t *p,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,p);
	setA(ap,p);
	va_end(ap);
}
void cST::setf(const char *p,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,p);
	setA(ap,p);
	va_end(ap);
}
void cST::insert(int iPos,const char *fmt,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	cST V(ap,fmt);
	if (V.iLen > 0)
	{
		if (iPos > iLen) iPos = iLen;
		insDel(iPos,V.length());
		memcpy(sBuf+iPos,V.sBuf,V.iLen);
	}
}

void cST::setA(va_list ar,const char *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	va_list args;
	int iRes;
	iLen = strlen(p) * 2;
	if (iLen < 128) iLen = 128;
	if (iLenLoc > iLen + 25) iLen = iLenLoc - 25;
	if (sWBuf){delete sWBuf; sWBuf = NULL;}
	//iLenLoc = iLen + 10;
	//if (sBuf ){delete sBuf;  sBuf  = NULL;}
	if (testUtf8(p,iLen)){ isUTF8 = true; p+=3; }	// 0xEF 0xBB 0xBF (utf-8 start)
	while (1){
		va_copy(args,ar);
		if (iLen + 25 > iLenLoc)
		{
			if (sBuf) delete sBuf;
			iLenLoc = ((iLen + 25) | 0xFF) + 1;
			sBuf = new char[iLenLoc];
			iLen = iLenLoc - 25;
		}
		//if (sBuf) delete sBuf;
		//iLen += 256 + (iLen >> 4);
		//iLenLoc = iLen + 25;
		//iLenLoc = (iLenLoc | 0xFF) + 1;
		//sBuf = new char[iLenLoc];
		sBuf[iLen + 4] = (char)0x5A;
		sBuf[iLen + 5] = (char)0xA5;
		sBuf[iLen + 6] = (char)0xAA;
		sBuf[iLen + 7] = (char)0x55;
#if defined(_WIN32)
		iRes = _vsnprintf(sBuf,iLen,p,args);
#elif defined(__linux__) || defined(__APPLE__)
		iRes=vsnprintf(sBuf,iLen,p,args);
#else
#  error unsupported os
#endif
		if ((sBuf[iLen + 4] != (char)0x5A) || (sBuf[iLen + 5] != (char)0xA5) || (sBuf[iLen + 6] != (char)0xAA) || (sBuf[iLen + 7] != (char)0x55)) _lxBreak_();

		if (iRes >= iLen){iLen=iRes; iRes=-1;}	// C99 geeft aan dat benodigde lengte hier word weergegeven
		if (iRes >= 0) { iLen = iRes; return;}
		if ((_strTlMaxBufLen>0)&&(iLen>_strTlMaxBufLen)) throw "maximum alloc size reached";
		iLen += 256 + (iLen >> 4);
	}
}
void cST::setA(va_list ar,const wchar_t *p)
{
	if (p == NULL) return;
	if (_uid != _StrDataUidC_) _lxBreak_();
	va_list args;
	int iRes;
	iLen = wcslen(p) * 2;
	if (iLen < 128) iLen = 128;
	if (iLenLoc > iLen + 25) iLen = iLenLoc - 25;
	//iLenLoc = iLen + 10;
	//if (sWBuf){delete sWBuf; sWBuf = NULL;}
	if (sBuf ){delete sBuf;  sBuf  = NULL;}
	while (1){
		va_copy(args,ar);
		if (iLen + 25 > iLenLoc)
		{
			if (sWBuf) delete sWBuf;
			iLenLoc = ((iLen + 25) | 0xFF) + 1;
			sWBuf = new wchar_t[iLenLoc];
			iLen = iLenLoc - 25;
		}
		//if (sWBuf) delete sWBuf;
		//iLen += 256 + (iLen >> 4);
		//iLenLoc = iLen + 25;
		//iLenLoc = (iLenLoc | 0xFF) + 1;
		//sWBuf = new wchar_t[iLenLoc];

		sWBuf[iLen + 4] = 0x5A;
		sWBuf[iLen + 5] = 0xA5;
		sWBuf[iLen + 6] = 0xAA;
		sWBuf[iLen + 7] = 0x55;

#if defined(_WIN32)
		iRes = _vsnwprintf((WCHAR*)sWBuf,iLen,(WCHAR*)p,args);
#elif defined(__linux__) || defined(__APPLE__)
#  ifdef __GLIBC_PREREQ
		iRes=vswprintf((wchar_t*)sWBuf,iLen,(wchar_t*)p,args);
#  else		// hele oude linux versies hebben geen vswprintf
		iRes = 1;
		_lxBreak_();
#  endif
#else
#  error unsupported os
#endif
		if ((sWBuf[iLen + 4] != 0x5A) || (sWBuf[iLen + 5] != 0xA5) || (sWBuf[iLen + 6] != 0xAA) || (sWBuf[iLen + 7] != 0x55)) _lxBreak_();

		if (iRes >= iLen){iLen=iRes; iRes=-1;}	// C99 geeft aan dat benodigde lengte hier word weergegeven
		if (iRes >= 0) { iLen = iRes; return;}
		if ((_strTlMaxBufLen>0)&&(iLen>_strTlMaxBufLen)) throw "maximum alloc size reached";
		iLen += 256 + (iLen >> 4);
	}
}

/* Ensure only single occurrences of char
 *  chr   char te remove from data
 * Return new data length
 */
int cST::single(char chr)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if (sBuf)
	{
		bool hit = false;
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			if (sBuf[i] == chr)
			{
				if (hit) continue;
				hit = true;
			}else hit = false;
			sBuf[j++] = sBuf[i];
		}
		sBuf[j] = 0;
		iLen = j;
	}
	return iLen;
}

/* Ensure only single occurrences of char
 *  chr   char te remove from data
 * Return new data length
 */
int cST::single(wchar_t chr)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch16bit();
	if (sWBuf)
	{
		bool hit = false;
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			if (sWBuf[i] == chr)
			{
				if (hit) continue;
				hit = true;
			}else hit = false;
			sWBuf[j++] = sWBuf[i];
		}
		sWBuf[j] = 0;
		iLen = j;
	}
	return iLen;
}
/* Remove all occurrences of char ch
 *  chr   char te remove from data
 * Return new data length
 */
int cST::remove(char chr)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if (sBuf)
	{
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			if (sBuf[i] == chr) continue;
			sBuf[j++] = sBuf[i];
		}
		sBuf[j] = 0;
		iLen = j;
	}
	return iLen;
}

/* Remove all occurrences of char ch
 *  chr   char te remove from data
 * Return new data length
 */
int cST::remove(wchar_t chr)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch16bit();
	if (sWBuf)
	{
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			if (sWBuf[i] == chr) continue;
			sWBuf[j++] = sWBuf[i];
		}
		sWBuf[j] = 0;
		iLen = j;
	}
	return iLen;
}

/* Remove all occurrences of chars
 *  sRemChs chars te remove from data
 * Return new data length
 */
int cST::removeMult(const char *sRemChs)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((sBuf) && (sRemChs) && (sRemChs[0]))
	{
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			char ch = sBuf[i];
			if (strchr(sRemChs,ch)) continue;
			sBuf[j++] = ch;		// char may stay
		}
		iLen=j;
		sBuf[j]=0;
	}
	return iLen;
}

/* Leave all occurrences of chars
 *  sRemChs chars te leve in the data
 * Return new data length
 */
int cST::leaveMult(const char *sHoldChs)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((sBuf) && (sHoldChs) && (sHoldChs[0]))
	{
		int i = 0, j = 0;
		for (; i < iLen; i++)
		{
			char ch = sBuf[i];
			if (strchr(sHoldChs,ch)) 
				sBuf[j++] = ch;		// char may stay
		}
		iLen=j;
		sBuf[j]=0;
	}
	return iLen;
}




/* Insert or delete a number of characters at a specifiek position
 * This function is safe against mistakes
 *  iPos  Position in dat to start at
 *  iLen  Number of bytes to delete (<0) or insert (>0)
*/
void cST::insDel(int iPos,int iLenAdd)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (iLenAdd==0) return;
	if (iPos < 0) {printf("Warning cSTP::insDel called with negative (%d) offset\n",iPos); iPos=0;}
	if (iPos > iLen) iPos = iLen;							// start pos was after end of string
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled

	if (iLenAdd < 0)	// need to delete characters
	{
		if (iPos >= iLen) return;		// nothing to do
		if (iPos - iLenAdd > iLen)	// delete less than requested
			iLenAdd = iPos - iLen;	// Calculate max data to delete
		if (iLenAdd >= 0) return;
		int iSize = iLen - iPos + iLenAdd;
		if (iSize > 0)
		{
			if (sWBuf)
			{
				memmove(sWBuf + iPos, sWBuf + iPos - iLenAdd, iSize * sizeof(wchar_t));
			}else if (sBuf)
			{
				memmove(sBuf + iPos, sBuf + iPos - iLenAdd, iSize * sizeof(char));
			}
		}
		iLen += iLenAdd;
		if (sWBuf) sWBuf[iLen] = 0;
		if (sBuf ) sBuf [iLen] = 0;
		return;
	}

	// Need to insert characters 
	if ((_strTlMaxBufLen > 0) && (iLen + iLenAdd > _strTlMaxBufLen)) _strSafeExit(_StTlErrMaxMemUsed_);
	
	// tested this setup and is only gives significant effects on stlAppend family functions
	if (iLen + iLenAdd + 1 > iLenLoc)	// not enough memory expand my memory
	{
		iLenLoc = iLen + iLenAdd + 256;
		iLenLoc+= iLenLoc / 2;				// 50% extra memory for future expansions
		iLenLoc = (iLenLoc | 0xFF) + 1;		// altijd veelvoud van 256 bytes
		if (sWBuf)
		{
			wchar_t *nb = new wchar_t[iLenLoc];
			if (nb == NULL) _strSafeExit(_StTlErrOutOfMem_);
			memcpy(nb,sWBuf,iLen * sizeof(wchar_t));
			delete sWBuf;
			sWBuf = nb;
			//sWBuf = (wchar_t*)realloc(sWBuf, iLenLoc * sizeof(wchar_t));
			//if (sWBuf==NULL) _strSafeExit(_StTlErrOutOfMem_);
		}else
		{
			char *nb = new char[iLenLoc];
			if (nb == NULL) _strSafeExit(_StTlErrOutOfMem_);
			if (sBuf)
			{
				memcpy(nb,sBuf,iLen * sizeof(char));
				delete sBuf;
			}
			sBuf = nb;
			//sBuf = (char*)realloc(sBuf, iLenLoc * sizeof(char));
			//if (sBuf==NULL) _strSafeExit(_StTlErrOutOfMem_);
		}
	}
	int iSize = iLen + 1 - iPos;	// calculate number of characters to move
	iLen += iLenAdd;				// new length 
	if (iSize > 0){
		if (sWBuf)
		{
			memmove(sWBuf + iPos + iLenAdd, sWBuf + iPos, iSize);
		}else if (sBuf)
		{
			memmove(sBuf + iPos + iLenAdd, sBuf + iPos, iSize);
		}
	}
	if (sWBuf) sWBuf[iLen] = 0;
	if (sBuf ) sBuf [iLen] = 0;
}

void cST::append(cST &p)
{
	append(&p);
}

void cST::append(cST *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	if (p->length() <= 0) return;		// niets te doen
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	if (sWBuf)
	{
		int iPos = iLen;
		insDel(iPos,p->length());
		memcpy(sWBuf + iPos, p->bufW(), p->length() * sizeof(wchar_t));
	}else
	{
		int iPos = iLen;
		insDel(iPos,p->length());
		memcpy(sBuf + iPos, p->buf(), p->length() * sizeof(char));
	}
};
void cST::append(STP p, bool consume /* = false */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p) append(p->iLen,p->sBuf);
	if (consume) stlFree(p);
}
void cST::append(int len,const wchar_t *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((len <= 0) || (p == NULL)) return;
	switch16bit();
	int iPos = iLen;
	insDel(iPos,len);
	memcpy(sWBuf + iPos, p, len * sizeof(wchar_t));
	sWBuf[iLen] = 0;
}
void cST::append(int len,const char  *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((len <= 0) || (p == NULL)) return;

	if (testUtf8(p,len)){ isUTF8 = true; p+=3; len -= 3; }	// 0xEF 0xBB 0xBF (utf-8 start)

	switch8bit();
	int iPos = iLen;
	insDel(iPos,len);
	memcpy(sBuf + iPos, p, len * sizeof(char));
	sBuf[iLen] = 0;
}
void cST::append(const wchar_t *p,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	va_list ap;
	va_start(ap,p);
	appendA(ap,p);
	va_end(ap);
}
void cST::append(const char  *p,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	va_list ap;
	va_start(ap,p);
	appendA(ap,p);
	va_end(ap);
}
void cST::appendA(va_list ar,const char *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	cST S(ar,p);
	append(&S);
}
void cST::appendA(va_list ar,const wchar_t *p)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (p == NULL) return;
	cST S(ar,p);
	append(&S);
}

int cST::convert(const char *sSrc,const char *sRep)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if (sRep == NULL) sRep = "";
	if ((sBuf == NULL)||(sSrc==NULL)||(sSrc[0]==0)) return 0;
	int iHit=0;
	int iSrc=strlen(sSrc);
	int iRep=strlen(sRep);
	if (iSrc>=iRep){						// We don't need extra room
		char *pS,*pR;
		pS=pR=sBuf;							// data buffer
		for (int i = iLen; i > 0; i--){
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
		iLen -= (iHit*(iSrc-iRep));			// Calculate new data length
	}else{									// We need to expand the data
		char *pS,*pR;
		pS=pR=sBuf;							// data buffer
		for (int i = iLen; i > 0; i--){		// Lets count the number of times we have a hit
			if ((i>=iSrc)&&(memcmp(sSrc,pS,iSrc)==0)){	// Found what we are looking for
				pS+=iSrc;
				i -=iSrc-1;
				iHit++;
				continue;
			}
			pS+=1;
		}
		if (iHit>0){
			int i = iLen;					// beware originele lengte
			insDel(0,iHit*(iRep-iSrc));		// Make room at beginning of data
			pR=sBuf;						// data buffer
			pS=pR+(iHit*(iRep-iSrc));		// Start offset to search
			for (; i > 0; i--){
				if ((i>=iSrc)&&(memcmp(sSrc,pS,iSrc)==0)){	// Found what we are looking for
					memcpy(pR,sRep,iRep);	// Swap new data in place
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
	return iHit;
}

int cST::convert(int chZoek,int chRep)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	int iHit = 0;
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	if (sBuf)
	{
		for (int i = 0; i < iLen; i++)
			if (sBuf[i] == (char)chZoek){
				sBuf[i] = chRep;
				iHit++;
			}
	}
	if (sWBuf)
	{
		for (int i = 0; i < iLen; i++)
			if (sWBuf[i] == (wchar_t)chZoek){
				sWBuf[i] = chRep;
				iHit++;
			}
	}
	return iHit;
}

void cST::strip(int cStrip)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	if (sBuf)
	{
		int i = 0, k = 0;
		while (sBuf[i] == cStrip) i++;
		for (int j = 0; i < iLen; i++){
			if ((sBuf[j++] = sBuf[i]) == cStrip) continue;
			k = j;		// First filtered char
		}
		iLen    = k;
		sBuf[k] = 0;
	}else if (sWBuf)
	{
		int i = 0, k = 0;
		while (sWBuf[i] == (wchar_t)cStrip) i++;
		for (int j = 0; i < iLen; i++){
			if ((sWBuf[j++] = sWBuf[i]) == (wchar_t)cStrip) continue;
			k = j;		// First filtered char
		}
		iLen    = k;
		sWBuf[k] = 0;
	}
}


int cST::count(wchar_t cDlm)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	int iHit = 0;
	if (sBuf)
	{
		for (int i = 0; i < iLen; i++)
			if (sBuf[i] == (char)cDlm)
				iHit++;
	}else if (sWBuf)
	{
		for (int i = 0; i < iLen; i++)
			if (sWBuf[i] == cDlm)
				iHit++;
	}
	return iHit;
}
void cST::toUpper()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	if (sBuf)
	{
		strupr(sBuf);
	}else if (sWBuf)
	{
		wcsupr(sWBuf);
	}
}
void cST::toLower()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	if (sBuf)
	{
		strlwr(sBuf);
	}else if (sWBuf)
	{
		wcslwr(sWBuf);
	}
}

/* Get value with user delimiter
 *  iD1  index to get (neg count from the back)
 *  cDlm character to use as delimiter
 * Return SubPart of STP
*/
cST cST::getDlm(int iD1,wchar_t cDlm)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	cST res;
	int iEind  = iLen;
	int iStart = 0;
	if (sBuf)
	{
		_strTlGetOfs(sBuf,(char)cDlm,iD1,&iStart,&iEind);
		int len = iEind - iStart;
		if (len > 0) res.set(len, sBuf + iStart);
	}else if (sWBuf)
	{
		_strTlGetOfsW(sWBuf,cDlm,iD1,&iStart,&iEind);
		int len = iEind - iStart;
		if (len > 0) res.set(len, sWBuf + iStart);
	}
	return res;
}
void cST::delDlm(int iD1,wchar_t cDlm)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if ((sWBuf) && (sBuf)){ delete sBuf; sBuf = NULL; }		// default to 16 bit when both buffers filled
	int iEind  = iLen;
	int iStart = 0;
	if (sBuf)
	{
		_strTlGetOfs(sBuf,(char)cDlm,iD1,&iStart,&iEind);
		if (iD1){
			if ((iStart>0)&&(sBuf[iStart-1]==(char)cDlm))	// check to see if prev or next char need to be deleted
				iStart--;	
			else 
				iEind++; 
		}
		insDel(iStart,iStart-iEind);
	}else if (sWBuf)
	{
		_strTlGetOfsW(sWBuf,cDlm,iD1,&iStart,&iEind);
		if (iD1){
			if ((iStart>0)&&(sWBuf[iStart-1]==cDlm))		// check to see if prev or next char need to be deleted
				iStart--;	
			else 
				iEind++; 
		}
		insDel(iStart,iStart-iEind);
	}
}

bool cST::compare(ansStl::cST &scmp)
{
	return compare(&scmp);
}
bool cST::compare(ansStl::cST *scmp)
{
	if (scmp == NULL) return false;
	if (scmp->iLen != iLen) return false;
	if ((scmp->sBuf) && (sBuf))
		return (strncmp(scmp->sBuf,sBuf,iLen) == 0);
	if ((scmp->sWBuf) && (sWBuf))
		return (memcmp(scmp->sWBuf,sWBuf,iLen * 2) == 0);
	return false;
}

bool cST::compare(const char *scmp)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((scmp == NULL) && (sBuf == NULL)) return true;
	if ((sBuf) && (scmp)) return (strcmp(sBuf,scmp) == 0);
	return false;
}
bool cST::compare(const char *scmp,size_t len)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((scmp == NULL) && (sBuf == NULL)) return true;
	if ((sBuf) && (scmp)) return (strncmp(sBuf,scmp,len) == 0);
	return false;
}

bool cST::comparei(const char *scmp)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((scmp == NULL) && (sBuf == NULL)) return true;
	if ((sBuf) && (scmp)) return (stricmp(sBuf,scmp) == 0);
	return false;
}

bool cST::StartsWith(const char *scmp,bool caseSensitive /* = true */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((scmp == NULL) && (sBuf == NULL)) return true;
	if ((scmp) && (sBuf))
	{
		int cmpLen = strlen(scmp);
		if (cmpLen > iLen) return false;
		if (caseSensitive)
			return (strncmp(scmp,sBuf,cmpLen) == 0);
		return (strnicmp(scmp,sBuf,cmpLen) == 0);
	}
	return false;
}
bool cST::EndsWith(const char *scmp,bool caseSensitive /* = true */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((scmp == NULL) && (sBuf == NULL)) return true;
	if ((scmp) && (sBuf))
	{
		int cmpLen = strlen(scmp);
		if (cmpLen > iLen) return false;
		if (caseSensitive)
			return (strncmp(scmp,sBuf + iLen - cmpLen,cmpLen) == 0);
		return (strnicmp(scmp,sBuf + iLen - cmpLen,cmpLen) == 0);
	}
	return false;

}

bool cST::StartsWith(const wchar_t *scmp,bool caseSensitive /* = true */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch16bit();
	if ((scmp == NULL) && (sWBuf == NULL)) return true;
	if ((scmp) && (sWBuf))
	{
		int cmpLen = wcslen(scmp);
		if (cmpLen > iLen) return false;
		if (caseSensitive)
			return (wcsncmp(scmp,sWBuf,cmpLen) == 0);
		return (_wcsnicmp(scmp,sWBuf,cmpLen) == 0);
	}
	return false;
}
bool cST::EndsWith(const wchar_t *scmp,bool caseSensitive /* = true */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch16bit();
	if ((scmp == NULL) && (sWBuf == NULL)) return true;
	if ((scmp) && (sWBuf))
	{
		int cmpLen = wcslen(scmp);
		if (cmpLen > iLen) return false;
		if (caseSensitive)
			return (wcsncmp(scmp,sWBuf + iLen - cmpLen,cmpLen) == 0);
		return (_wcsnicmp(scmp,sWBuf + iLen - cmpLen,cmpLen) == 0);
	}
	return false;
}


bool cST::contains(const char *sZoek)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if ((sZoek == NULL) && (sBuf == NULL)) return true;
	if ((sBuf) && (sZoek))return (strstr(sBuf,sZoek) != NULL);
	return false;
}

/*
**  Case-sensitive version
*/

static int patmat (const char *pat, const char *str)
{
	switch (*pat)
	{
	case '\0': return !*str;
	case '*' : return patmat(pat+1, str) || (*str && patmat(pat, str+1));
	case '?' : return *str && patmat(pat+1, str+1);
	default  : return (*pat == *str) && patmat(pat+1, str+1);
	}
}

static int xstrcmp(const char *pat, const char *str)
{
	if (NULL == str || NULL == pat) return -1;
	return(patmat(pat, str));
}

bool cST::compareMatch(const char *match)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if (sBuf) return (xstrcmp(match,sBuf) > 0);
	return false;
}

/*
**  Case-insensitive version
*/
static int patimat(const char *pat, const char *str)
{
	switch (*pat)
	{
	case '\0': return !*str;
	case '*' : return patimat(pat+1, str) || (*str && patimat(pat, str+1));
	case '?' : return *str && patimat(pat+1, str+1);
	default  : return (toupper(*pat) == toupper(*str)) && patimat(pat+1, str+1);
	}
}

static int xstricmp(const char *pat, const char *str)
{
	if (NULL == str || NULL == pat) return -1;
	return(patimat(pat, str));
}

bool cST::compareMatchI(const char *match)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	switch8bit();
	if (sBuf) return (xstricmp(match,sBuf) > 0);
	return false;
}

int cST::getDlmInt(int iD1,wchar_t cDlm)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return getDlm(iD1,cDlm).toInt();
}

double cST::getDlmDbl(int iD1,wchar_t cDlm)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	return getDlm(iD1,cDlm).toDouble();
}

int cST::toInt()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	// fix voor als waarde 0009 is (word als octaal gezien)
	if (sBuf)
	{
		int iOfs = 0;
		while (sBuf[iOfs] == '0') iOfs++;
		if ((iOfs > 0) && (sBuf[iOfs] == 'x')) iOfs--;
		if ((iOfs > 0) && (sBuf[iOfs] == 'X')) iOfs--;
		if ((iOfs > 0) && (sBuf[iOfs] == 0  )) iOfs--;
		return strtol(sBuf + iOfs,NULL,0);
	}
	if (sWBuf){
		int iOfs = 0;
		while (sWBuf[iOfs] == '0') iOfs++;
		if ((iOfs > 0) && (sWBuf[iOfs] == 'x')) iOfs--;
		if ((iOfs > 0) && (sWBuf[iOfs] == 'X')) iOfs--;
		if ((iOfs > 0) && (sWBuf[iOfs] == 0  )) iOfs--;
		return wcstol(sWBuf + iOfs,NULL,0);
	}
	return 0;
}
double cST::toDouble()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (sBuf) return strtod(sBuf,NULL);
	if (sWBuf) return wcstod((wchar_t *)sWBuf,NULL);
	return 0.0;
}

void  cST::setDlm(int iD1,wchar_t cDlm,const char *fmt,...)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	cST v(ap,fmt);
	va_end(ap);
	setDlm(iD1,cDlm,&v);
}
void  cST::setDlm(int iD1,wchar_t cDlm,int iValue)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	cST v("%d",iValue);
	setDlm(iD1,cDlm,&v);
}
void  cST::setDlm(int iD1,wchar_t cDlm,STP value,bool consume /* = false */)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	cST v(value,consume);
	setDlm(iD1,cDlm,&v);
}
void  cST::setDlm(int iD1,wchar_t cDlm,cST* value)
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (value == NULL) return;
	int iStart= 0;
	int iEind = iLen;
	if (sWBuf)
	{
		cST *va_16 = NULL;
		if (!value->is16bit())			// als geen 16 bit is dan 16 bit maken (copy)
		{
			va_16 = new cST(*value);
			va_16->switch16bit();
			value = va_16;
		}
		switch16bit();	// cleanup 8 bit data
		iD1 = _strTlGetOfsW(sWBuf, cDlm, iD1, &iStart, &iEind);
		int iStoreLen = value->length();
		int iOldLen = iEind - iStart;
		int iNewLen = iStoreLen + abs(iD1);
		insDel(iStart, iNewLen - iOldLen);
		while (iD1>0){sWBuf[iStart++] = cDlm; iD1--;}
		if (iStoreLen){memcpy(sWBuf + iStart, value->bufW(), iStoreLen); iStart += iStoreLen;}
		while (iD1<0){sWBuf[iStart++] = cDlm; iD1++;}
		if (va_16) delete va_16;
	}else if (sBuf)
	{
		cST *va_8 = NULL;
		if (!value->is8bit())			// als geen 8 bit is dan 8 bit maken (copy)
		{
			va_8 = new cST(*value);
			va_8->switch16bit();
			value = va_8;
		}
		switch8bit();
		iD1 = _strTlGetOfs(sBuf,(char)cDlm, iD1, &iStart, &iEind);
		int iStoreLen = value->length();
		int iOldLen = iEind - iStart;
		int iNewLen = iStoreLen + abs(iD1);
		insDel(iStart, iNewLen - iOldLen);
		while (iD1>0){sBuf[iStart++] = (char)cDlm; iD1--;}
		if (iStoreLen){memcpy(sBuf + iStart, value->buf(), iStoreLen); iStart += iStoreLen;}
		while (iD1<0){sBuf[iStart++] = (char)cDlm; iD1++;}
		if (va_8) delete va_8;
	}else set(value->length(),value->bufW());
}




void cST::update16bit()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (sWBuf) return;
	if (sBuf == NULL) return;
	sWBuf = new wchar_t[iLenLoc];
	if (isUTF8)
	{
		int iOfsS = 0;
		int iOfsD = 0;
		while (iOfsS < iLen)
		{
			wchar_t ch = (sBuf[iOfsS++] & 0xFF);
			if ((ch & 0x80) ==0)
			{
				sWBuf[iOfsD++] = ch;
				continue;
			}
			int iExtra = 1;
			if ((ch & 0xE0) == 0xC0)
			{
				ch &= 0x1F;
			}else if ((ch & 0xF0) == 0xE0)
			{
				ch &= 0x0F;
				iExtra = 2;
			}else if ((ch & 0xF8) == 0xF0)
			{
				ch &= 0x07;
				iExtra = 3;
			}else
			{
#ifdef _DEBUG
				throw "geen juiste UTF-8 string";
#else
				printf("#90# geen juiste UTF-8 string");
				sWBuf[iOfsD++] = ch;
				continue;
#endif
			}
			while (iExtra-- > 0)
			{
				if (iOfsS > iLen)
				{
#ifdef _DEBUG
					throw "premature einde van UTF-8 string";
#else
					printf("#90# premature einde van UTF-8 string");
					break;
#endif
				}
				wchar_t t = (sBuf[iOfsS++] & 0xFF);
				if ((t & 0xC0) != 0x80)
				{
#ifdef _DEBUG
					throw "ongeldige UTF-8 string extension";
#else
					printf("#90# ongeldige UTF-8 string extension");
					break;
#endif
				}
				ch = (ch<<6) | (t & 0x3F);
			}
			sWBuf[iOfsD++] = ch;
		}
		iLen = iOfsD;
		while (iOfsD < iLenLoc) sWBuf[iOfsD++] = 0;
		delete sBuf;
		sBuf = NULL;
	}else
	{
		for (int i = 0; i < iLenLoc; i++) sWBuf[i] = (sBuf[i] & 0xFF);
	}
}
void cST::update8bit()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	if (sBuf) return;
	if (sWBuf == NULL) return;
	if (!isUTF8)			// test if we need to become UTF-8 formated
		for (int i = 0; i < iLen; i++)
			if (sWBuf[i] > 0xFF){isUTF8 = true; break;}

	if (isUTF8)
	{
		int iExtra = 0;
		for (int i = 0; i < iLen; i++)
		{
			wchar_t ch = sWBuf[i];
			if (ch < 0x80) continue;
			if (ch < 0x800){ iExtra += 1; continue; }
			//if (ch < 0x10000){ iExtra += 2; continue; }	// only utf-32
			iExtra += 2;
		}
		if (iLen + iExtra + 4 > iLenLoc) iLenLoc = ((iLen + iExtra + 4) | 0xFF) + 1;
		sBuf = new char[iLenLoc];
		int iOfsS = 0;
		int iOfsD = 0;
		while (iOfsS < iLen)
		{
			wchar_t ch = sWBuf[iOfsS++];
			if (ch < 0x80)
			{
				sBuf[iOfsD++] = (ch & 0xFF);
				continue;
			}
			if (ch < 0x800)
			{
				sBuf[iOfsD++] = (ch >> 6) | 0xC0;
				sBuf[iOfsD++] = ((ch >> 0) & 0x3F) | 0x80;
				continue;
			}
			sBuf[iOfsD++] = (ch >> 12) | 0xE0;
			sBuf[iOfsD++] = ((ch >> 6) & 0x3F) | 0x80;
			sBuf[iOfsD++] = ((ch >> 0) & 0x3F) | 0x80;
		}
		iLen = iOfsD;
		while (iOfsD < iLenLoc) sBuf[iOfsD++] = 0;
		delete sWBuf;
		sWBuf = NULL;
	}else
	{
		sBuf = new char[iLenLoc];
		for (int i = 0; i < iLenLoc; i++) sBuf[i] = (sWBuf[i] & 0xFF);
	}
}
void cST::switch8bit()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	update8bit();
	if (iLenLoc <= 0)
	{
		iLenLoc = 256;
		sBuf = new char[iLenLoc];
		memset(sBuf,0,sizeof(char) * iLenLoc);
	}
	if (sWBuf){delete sWBuf; sWBuf = NULL;}
}
void cST::switch16bit()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	update16bit();
	if (iLenLoc <= 0)
	{
		iLenLoc = 256;
		sWBuf = new wchar_t[iLenLoc];
		memset(sWBuf,0,sizeof(wchar_t) * iLenLoc);
	}
	if (sBuf){delete sBuf; sBuf = NULL;}
}
const char * cST::buf()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	update8bit();
	return sBuf;
}
const wchar_t * cST::bufW()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	update16bit();
	return sWBuf;
}

STP cST::getStp()
{
	if (_uid != _StrDataUidC_) _lxBreak_();
	update8bit();
	return stlInitLenSrc(iLen,sBuf);
}


