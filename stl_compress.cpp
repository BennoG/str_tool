#include <stdlib.h>

#include "stl_str.h"
#ifdef __linux__
#  include <zlib.h>
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/

#ifdef _WIN32
  typedef unsigned long (CDECL *fpCompressBound)(unsigned long sourceLen); 
  typedef int           (CDECL *fpCompress2    )(unsigned char *dst, unsigned long *destLen,const unsigned char *source, unsigned long sourceLen,int level);
  typedef int           (CDECL *fpUncompress   )(unsigned char *dst, unsigned long *destLen,const unsigned char *source, unsigned long sourceLen);
#endif

class compressZlib
{
private:
#ifdef _WIN32
	HINSTANCE m_hDll;
	fpCompressBound compressBound;
	fpCompress2     compress2;
	fpUncompress    uncompress;
#endif
public:
	compressZlib();
	~compressZlib();
	STP stlCompress(STP src,int level = 6);
	STP stlCompress(void *src,unsigned long srcLen,int level = 6);
	STP stlUncompress(STP src);
};

compressZlib::compressZlib()
{
#ifdef _WIN32
	m_hDll = LoadLibrary("zlib1.dll");
	if (m_hDll != NULL)
	{
		compressBound = (fpCompressBound)GetProcAddress(m_hDll,"compressBound");
		compress2     = (fpCompress2    )GetProcAddress(m_hDll,"compress2");
		uncompress    = (fpUncompress   )GetProcAddress(m_hDll,"uncompress");
	}else
	{
		compressBound = NULL;
		compress2     = NULL;
		uncompress    = NULL;
	}
#endif
}
compressZlib::~compressZlib()
{
#ifdef _WIN32
	if (m_hDll)
	{
		FreeLibrary(m_hDll);
		m_hDll = NULL;
		compressBound = NULL;
		compress2     = NULL;
		uncompress    = NULL;
	}
#endif
}

STP compressZlib::stlCompress(STP src,int level /* = 6 */)
{
	if (src == NULL) return NULL;
	return stlCompress(src->sBuf,src->iLen,level);
}

STP compressZlib::stlCompress(void *src,unsigned long srcLen,int level /* = 6 */)
{
	if (level < 0) level = 6;
	if (level > 9) level = 9;
#ifdef _WIN32
	if (compressBound == NULL) return NULL;
	if (compress2     == NULL) return NULL;
#endif
	unsigned long len = compressBound((unsigned long)srcLen);
	STP sRes = stlInitLen(len,0);
	int iRes = compress2((unsigned char *)(sRes->sBuf),&len,(unsigned char *)src,srcLen,level);
	if (iRes == 0)
	{
		sRes->iLen = len;
		return sRes;
	}
	stlFree(sRes);
	return NULL;
}
STP compressZlib::stlUncompress(STP src)
{
	if (src == NULL) return NULL;
#ifdef _WIN32
	if (uncompress == NULL) return NULL;
#endif
	unsigned long len = src->iLen * 5;
	while (true)
	{
		STP sRes = stlInitLen(len,0);
		int iRes = uncompress((unsigned char *)(sRes->sBuf),&len,(unsigned char *)(src->sBuf),src->iLen);
		if (iRes == 0)
		{
			sRes->iLen = len;
			return sRes;
		}
		stlFree(sRes);
		if (iRes == -5){len = len * 3; continue;}	// te kort ruimte in output buffer
		break;
	}
	return NULL;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

static compressZlib *engZlib = NULL;

STP stlCompressZlib(STP src,int level = 6)
{
	if (engZlib == NULL) engZlib = new compressZlib();
	return engZlib->stlCompress(src,level);
}
STP stlCompressZlib(void *src,unsigned long srcLen,int level = 6)
{
	if (engZlib == NULL) engZlib = new compressZlib();
	return engZlib->stlCompress(src,srcLen,level);
}
STP stlUnompressZlib(STP src)
{
	if (engZlib == NULL) engZlib = new compressZlib();
	return engZlib->stlUncompress(src);
}

