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

#include "stl_str.h"

void _stpWinPrintError(void)
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



static STP stlRegSplit(HKEY *hKey,char *sFull)
{
	HKEY kRt=((HKEY)-1);
	STP sRoot,sName;
	sName=stlSetSt(sFull);
	stlConvert(sName,'/','\\');
	sRoot=stlGetDlm(sName,1,'\\');
	strupr(sRoot->sBuf);
	if (strcmp(sRoot->sBuf,"HKEY_CLASSES_ROOT")==0){
		kRt=HKEY_CLASSES_ROOT;
	}else if (strcmp(sRoot->sBuf,"HKEY_CURRENT_USER")==0){
		kRt=HKEY_CURRENT_USER;
	}else if (strcmp(sRoot->sBuf,"HKEY_LOCAL_MACHINE")==0){
		kRt=HKEY_LOCAL_MACHINE;
	}else if (strcmp(sRoot->sBuf,"HKEY_USERS")==0){
		kRt=HKEY_USERS;
	}else if (strcmp(sRoot->sBuf,"HKEY_PERFORMANCE_DATA")==0){
		kRt=HKEY_PERFORMANCE_DATA;
	}else if (strcmp(sRoot->sBuf,"HKEY_CURRENT_CONFIG")==0){
		kRt=HKEY_CURRENT_CONFIG;
	}else if (strcmp(sRoot->sBuf,"HKEY_DYN_DATA")==0){
		kRt=HKEY_DYN_DATA;
	}else{
		stlFree(sName);
		stlFree(sRoot);
		return NULL;
	}
	stlFree(sRoot);
	*hKey=kRt;
	stlDelDlm(sName,1,'\\');
	return sName;
}

/* Get value from registry
 *	 sKey   key to registry (full path)
 *   sName  Name of registry value
 * Return NULL error
 *        STP  contens of reg value
 */
STP stlGetRegVal(char *sKey,char *sName)
{
	DWORD dType;
	int iMin=-1;
	//char sVal[200];
	HKEY kRt,hKey;
	STP sPath,sRes=NULL,sTmp;
	sPath=stlRegSplit(&kRt,sKey);
	if (sPath==NULL) return NULL;

	if (RegOpenKeyEx(kRt,sPath->sBuf,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS){
		if (RegQueryValueEx(hKey,TEXT(sName),NULL,&dType,NULL,&iMin) != ERROR_SUCCESS) iMin=-1;
		if (iMin>=0)
		{
			sRes=stlInitLen(iMin+5,0);
			if (RegQueryValueEx(hKey,TEXT(sName),NULL,&dType,sRes->sBuf,&iMin) == ERROR_SUCCESS){
				switch(dType)
				{
				case REG_EXPAND_SZ:
				case REG_SZ: 
					sRes->iLen=strlen(sRes->sBuf);
					break;
				case REG_MULTI_SZ:
					sRes->iLen=iMin;
					stlConvert(sRes,0,_D1);
					break;
				case REG_BINARY:
					sRes->iLen=iMin;
					break;
				case REG_DWORD:
				case REG_DWORD_BIG_ENDIAN:
					sTmp=stlSetStf("%d",*(int*)(sRes->sBuf));
					stlFree(sRes);
					sRes=sTmp;
					break;
				}
			}
		}
		RegCloseKey(hKey);
	}
	stlFree(sPath);
	return sRes;
}

/* Delete key in registry
 *	sKey    key to delete
 *  bUncond Auto remove subkeys
 * return 0 ok
 *        -1 error
 */
int stlDelRegKey(char *sKey,BOOL bUncond)
{
	int  iRes=-1;
	HKEY kRt,hKey;
	STP  sPath,sPat1;
	sPath=stlRegSplit(&kRt,sKey);
	if (sPath==NULL) return iRes;
	sPat1=stlGetDlm(sPath,-2,'\\');
	stlDelDlm(sPath,-2,'\\');
	if (RegOpenKeyEx(kRt,sPath->sBuf,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS){
		if (RegDeleteKey(hKey,sPat1->sBuf)==ERROR_SUCCESS)
			iRes=0;
		else
			_stpWinPrintError();
		RegCloseKey(hKey);
	}
	stlFree(sPath);
	stlFree(sPat1);
	return iRes;
}

/* Set value in registry (or delete value if pValue = NULL)
 *	sKey   Complete path to reg key
 *  sName  Name of value to set or delete
 *  pValue Value to write (NULL will delete value)
 *  dwType REG_DWORD REG_SZ REG_BINARY
 * return 0 ok
 *        -1 error
 */
int stlSetRegVal(char *sKey,char *sName,void *pValue,DWORD dwType)
{
	int  iRes=-1;
	HKEY kRt,hKey;
	STP  sPath;
	sPath=stlRegSplit(&kRt,sKey);
	if (sPath==NULL) return -1;

	if (pValue==NULL)
	{
		if (RegOpenKeyEx(kRt,sPath->sBuf,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS)
		{
			if (RegDeleteValue(hKey,TEXT(sName))==ERROR_SUCCESS) iRes=0;
			RegCloseKey(hKey);
		}
	}else
	{
		if (RegCreateKeyEx(kRt,sPath->sBuf,0,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL)==ERROR_SUCCESS)
		{
			switch (dwType){
				case REG_DWORD:
				case REG_DWORD_BIG_ENDIAN:
					if (RegSetValueEx(hKey,TEXT(sName),0,dwType,pValue,sizeof(DWORD ))==ERROR_SUCCESS) iRes=0;
					break;
				default:
					if (RegSetValueEx(hKey,TEXT(sName),0,dwType,pValue,strlen(pValue))==ERROR_SUCCESS) iRes=0;
					break;
			}
			RegCloseKey(hKey);
		}
	}
	stlFree(sPath);
	return iRes;
}

#endif // _WIN32
