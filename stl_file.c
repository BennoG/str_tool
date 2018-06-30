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

#if defined(__linux__) || defined(__APPLE__)
#  include <unistd.h>
#  include <sys/stat.h>
#else
#  include <io.h>
#  include <direct.h>
#endif

#include "stl_str.h"
#include "internal.h"
#include <errno.h>


#ifdef __linux__
/* Linux special 
 *	Read file from /proc/ file system (these don't support SEEK_END,SEEK_SET)
 *  so the size can not be determent at forehand.
 *  We read the file until we get an EOF
 *
 *	sFilename  Name of file to read
 *  iStart     >=0 start position where to start reading
 *  iBytes     <0  Read til end of file 
 *             >=0 Number of bytes to read from file
 */
static STP stlProcFileRead(const char *sFilename,int iStart,int iBytes)
{
	FILE *f1;
	STP sRes;
	int iOfs=0,ch;
	f1=fopen(sFilename,"rb");
	if (f1==NULL) return NULL;
	// Read past first bytes of file
	while (iStart>0){fgetc(f1);iStart--;}
	sRes=_strTlInitVar(250);
	while (1){
		if (iBytes>=0)
			if (--iBytes<0) 
				break;
		ch=fgetc(f1);
		if (ch==EOF) break;
		if (iOfs>=sRes->iLen) stlInsDel(sRes,iOfs,250);
		sRes->sBuf[iOfs++]=ch;
	}
	sRes->iLen=iOfs;
	sRes->sBuf[iOfs]=0;
	return sRes;
}
#endif

/* Read string struct from FILE data stream
 *	fIn      communication structure
 *  sIgnore  string of characters to ignore in data stream.           (optional)
 *  sStop    string of chars to stop on.                              (optional)
 * return: NULL End of file reached
 *         Data data was read til stop char was reached
 */
STP stlFileReadLineEx(FILE *fIn,const char *sIgnore,const char *sStop)
{
	int iOfs=0,iExtra=5,iRes;
	STP sLin;
	char *sBuf;
	if (fIn==NULL) return NULL;
	sLin=_strTlInitVar(iExtra);
	sBuf=sLin->sBuf;

	while (1){
		iRes=fgetc(fIn);
		if (iRes==EOF) break;
		if ((sIgnore) && (strchr(sIgnore,iRes))) continue;	// ignore char
		if ((sStop  ) && (strchr(sStop  ,iRes))) break;			// Abort char
		sBuf[iOfs++]=iRes;
		if (iOfs>=sLin->iLen){
			sLin->iLen=iOfs;					// Set actual count in struct
			sLin->sBuf[iOfs]=0;				// Add terminating 0
			iExtra *= 2;							// Expand some more
			_strTlInsDel(sLin,iOfs,iExtra);
			sBuf=sLin->sBuf;
		}
	}
	sLin->iLen=iOfs;					// Set actual count in struct
	sLin->sBuf[iOfs]=0;				// Add terminating 0
	if ((iOfs==0)&&(iRes==EOF)){stlFree(sLin); sLin=NULL;}
	return sLin;
}


STP stlFileReadLine(FILE *fIn)
{
	return stlFileReadLineEx(fIn,"\r","\n");	
}

/* Test if file exists
*	sFilename  Name of file to read
*  Return 0 no file
*         1 yes file exists
*/
int stlFileExist(const char *sFileName)
{
#ifdef __linux__
#  ifdef __arm__
	FILE *f1 = fopen(sFileName,"rb");
	if (f1){ fclose(f1); return 1; }
	return 0;
#  else	// __arm__  onderstaande werkt niet op samba drive op Linux arm (rpi)
	struct stat sts;
	if ((stat (sFileName, &sts)) == -1) return 0;
	return 1;
#  endif	// __arm__
#else
	if (_access(sFileName,0) == 0) return 1;
	return 0;
#endif
}

/* Read a file from some position with some length
 *	sFilename  Name of file to read
 *  iStart     >=0 start position where to start reading
 *             <0  Number of bytes from the end of the file (not for /proc/ on linux)
 *  iBytes     <0  Read til end of file 
 *             >=0 Number of bytes to read from file
 * Return NULL file does not exist of acces deny
 *        STP  Data read from file
 */
STP stlFileRead(const char *sFilename,int iStart,int iBytes)
{
	int iLen;
	STP sRes=NULL;
	FILE *f1;
#ifdef __linux__
	if (strncmp(sFilename,"/proc/",6)==0)
		return stlProcFileRead(sFilename,iStart,iBytes);
#endif
	f1=fopen(sFilename,"rb");
	if (f1==NULL) return NULL;
	fseek(f1,0,SEEK_END);
	iLen=ftell(f1);
	if (iStart<0)							// Read n bytes from end of file
	{
		iStart=iLen+iStart;			// Start pos in file
		if (iStart<0) iStart=0;	// Prevent pre start file read
	}else{
		if (iStart>iLen) iStart=iLen;
	}
	if (iBytes<0) iBytes=iLen-iStart;
	if (iBytes+iStart>iLen) iBytes=iLen-iStart;

	sRes=_strTlInitVar(iBytes);
	fseek(f1,iStart,SEEK_SET);
	iLen=fread(sRes->sBuf,1,iBytes,f1);
	fclose(f1);

	if ((iLen < sRes->iLen)&&(iLen>=0))
	{
		sRes->iLen=iLen;
		sRes->sBuf[iLen]=0;
	}
	return sRes;
}
/* Write data to file
 *  sData      data to write to file
 *  sFileName  name to use for file
 * Return 0 Ok
 *       -1 illegal filename
 *       -2 not all data could be written (disk full ??)
 */
int stlFileWrite(STP sData,const char *sFilename)
{
	int iRes=-99,iLen;
	FILE *f1;
	STP sBakName;

	if ((sFilename==NULL)||(sFilename[0]==0)) return -1;
	sBakName=stlSetSt(sFilename);
	stlAppendSt(sBakName,".bak");
	unlink(sBakName->sBuf);
	rename(sFilename,sBakName->sBuf);
	f1=fopen(sFilename,"wb");
	if (f1){
		iLen=fwrite(sData->sBuf,1,sData->iLen,f1);
		fclose(f1);
		if (iLen==sData->iLen) iRes=0; else iRes=-2;
	}else{
		unlink(sFilename);
		rename(sBakName->sBuf,sFilename);
	}
	stlFree(sBakName);
	return iRes;
}

#ifdef __linux__
static int _mkdir(const char * _Path)
{
	return mkdir(_Path,0755);
}
#endif

int stlMakeDirp(const char *sDirName)
{
	int iErr;
	if ((sDirName == NULL)||(sDirName[0] == 0)) return -2;
	iErr = _mkdir(sDirName);
	if (iErr < 0)
	{
		if (errno == EEXIST) return 0;
		if (errno == ENOENT)
		{
			char cLast;
			STP sDir = stlSetSt(sDirName);
#ifdef _WIN32
			char cDir = '\\';					// directory name separator
			stlConvert(sDir,'/',cDir);
#else
			char cDir = '/';					// directory name separator
			stlConvert(sDir,'\\',cDir);
#endif
			cLast = (sDir->iLen > 0 ? sDir->sBuf[sDir->iLen-1] : 0);	// last token of directory name
			stlDelDlm(sDir,-2,cDir);
			iErr = stlMakeDirp(sDir->sBuf);
			stlFree(sDir);
			if (iErr < 0) return iErr;
			if (cLast == cDir) return 0;		// last token was '\\' on Windows or '/' on Linux
			return _mkdir(sDirName);
		}
		return -1;
	}
	return 0;
}

int stlFileWriteDir(STP sData,const char *sDir,const char *sFilename)
{
	if ((sDir) && (sDir[0]))
	{
		int iRes;
		STP sFn = stlSetSt(sDir);
#ifdef _WIN32
		char cDir = '\\';
		stlConvert(sFn,'/',cDir);
#else
		char cDir = '/';
		stlConvert(sFn,'\\',cDir);
#endif
		if ((sFn->iLen) && (sFn->sBuf[sFn->iLen] != cDir)) stlAppendCh(sFn,cDir);
		stlAppendSt(sFn,sFilename);
		stlMakeDirp(sDir);
		iRes = stlFileWrite(sData,sFn->sBuf);
		stlFree(sFn);
		return iRes;
	}
	return stlFileWrite(sData,sFilename);
}
