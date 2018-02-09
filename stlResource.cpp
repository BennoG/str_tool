#ifdef _WIN32

#include "stl_str.h"
#include <winver.h>

using namespace ansStl;

#pragma comment(lib,"Version.lib")

void stlResource::load()
{
	DWORD dummy,dwSize;
	char szFilename[250];
	GetModuleFileName(NULL,szFilename,sizeof(szFilename)-2);
	// allocate a block of memory for the version info
	dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	wchar_t *buf = new wchar_t[dwSize + 10];
	GetFileVersionInfo(szFilename,0,dwSize,buf);
	for (DWORD i = 0; i < dwSize; i++)
		if ((buf[i] > 127) || (buf[i] < 32)) buf[i] = 0;
	resData.set(dwSize,buf);
	resData.single((wchar_t)0);
}

ansStl::cST stlResource::get(const char *name)
{
	int iCnt = resData.count(0) + 1;
	for (int i = 1; i <= iCnt; i++)
	{
		ansStl::cST v = resData.getDlm(i,(wchar_t)0);
		if (v.length() < 2) continue;
		if (stricmp(v.buf(),name)==0)
		{
			for (i = i + 1; i <= iCnt; i++)
			{
				v = resData.getDlm(i,(wchar_t)0);
				if (v.length() > 1) return v;
			}
		}
	}
	ansStl::cST res = "--";
	return res;
}

#endif // _WIN32

