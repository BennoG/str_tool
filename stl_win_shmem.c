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
#ifdef _WIN32

#include <windows.h>
#include <direct.h>

#include "stl_str.h"
#include "stl_thread.h"

#pragma hdrstop

#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
//#  define _lxBreak_() __asm int 3;
#endif

#define  _MaxShMaps_	20

static struct{
	char sKey[100];
	HANDLE hMap;
	void *pData;
}shHdlMap[_MaxShMaps_];

static stlMutex hdlMux=stlMutexInitializer;

static void _stlInitMaps(void)
{
	static int iDone=0;
	if (iDone) return;
	memset(shHdlMap,0,sizeof(shHdlMap));
}

static void _printError(void)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	printf("Error %s\n",lpMsgBuf);
	LocalFree( lpMsgBuf );
}

/* Init shared memory region
 *	sIdName    name to identify this shared memory
 *  iSize      Number of bytes to allocate
 * Reutrn NULL Error
 *        ptr  pointer to allocated memory
 */
void *stlShmemGet(const char *sIdName,int iSize)
{
	int iInit=0,i;
	void *pRes=NULL;
	if ((iSize<1)||(iSize>64*1024*1204)) return NULL;
	if (sIdName==NULL) return NULL;
	if (sIdName[0]==0) return NULL;
	if (strlen(sIdName)>99) return NULL;
	stlMutexLock(&hdlMux);
	_stlInitMaps();
	// Look first if already done same idname
	for (i=0;i<_MaxShMaps_;i++)
	{
		if (shHdlMap[i].hMap==NULL) continue;
		if (strcmp(sIdName,shHdlMap[i].sKey)==0)
		{
			pRes=shHdlMap[i].pData;
			break;
		}
	}
	if (pRes==NULL)
	{
		for (i=0;i<_MaxShMaps_;i++)
		{
			if (shHdlMap[i].hMap!=NULL) continue;
			shHdlMap[i].hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,sIdName);
			if (shHdlMap[i].hMap <= 0){
				shHdlMap[i].hMap=0;
//				printf("OpenFileMapping warning\n");
//				_printError();
			}
			if (shHdlMap[i].hMap <= 0){
				SECURITY_ATTRIBUTES SA;
				SECURITY_DESCRIPTOR SD;
				memset(&SA,0,sizeof(SA));
				memset(&SD,0,sizeof(SD));
				SA.nLength=sizeof(SA);
				SA.lpSecurityDescriptor=&SD;
				InitializeSecurityDescriptor(&SD,SECURITY_DESCRIPTOR_REVISION);
				SetSecurityDescriptorDacl (&SD,TRUE,NULL,FALSE);
				SetSecurityDescriptorGroup(&SD,NULL,FALSE);
				shHdlMap[i].hMap = CreateFileMapping(INVALID_HANDLE_VALUE,&SA,PAGE_READWRITE,0,iSize,sIdName);
				iInit=1;
			}
			if (shHdlMap[i].hMap <= 0){
				shHdlMap[i].hMap=0;
				printf("CreateFileMapping error\n");
				_printError();
				break;
			}
			shHdlMap[i].pData=MapViewOfFile(shHdlMap[i].hMap,FILE_MAP_ALL_ACCESS,0,0,iSize);
			if (shHdlMap[i].pData==NULL){
				printf("MapViewOfFile error\n");
				_printError();
				CloseHandle(shHdlMap[i].hMap);
				shHdlMap[i].hMap=NULL;
				break;
			}
			strcpy(shHdlMap[i].sKey,sIdName);
			pRes=shHdlMap[i].pData;
			if (iInit) memset(pRes,0,iSize);
			break;
		}
	}
	stlMutexUnlock(&hdlMux);
	return pRes;
}

/* Release shared memory region
 *	sIdName    name to identify this shared memory
 */
void stlShmemRelease(const char *sIdName)
{
	int i;
	if (sIdName==NULL) return;
	if (sIdName[0]==0) return;
	stlMutexLock(&hdlMux);
	_stlInitMaps();
	// Look first if already done same idname
	for (i=0;i<_MaxShMaps_;i++)
	{
		if (shHdlMap[i].hMap==NULL) continue;
		if (strcmp(sIdName,shHdlMap[i].sKey)==0)
		{
			UnmapViewOfFile(shHdlMap[i].pData);
			CloseHandle(shHdlMap[i].hMap);
			memset(&shHdlMap[i],0,sizeof(shHdlMap[i]));
		}
	}
	stlMutexUnlock(&hdlMux);
}

#endif // _WIN32
