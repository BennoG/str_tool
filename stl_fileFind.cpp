
#include "stl_str.h"
#include "stl_fileFind.h"

#if defined(__linux__) || defined(__APPLE__)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <stdio.h>
#endif
#include <vector>
#include <algorithm>


/*
**  XSTRCMP.C - Simple string pattern matching functions using DOS
**              wildcards ('?' & '*').
**
**  Derived from code by Arjan Kentner (submitted by Steve Summit),
**  modified by Bob Stout.
**  Arguments: 1 - Pattern to match
**             2 - String to match
**
**  Returns:  1  if match
**            0  if no match
**           -1  if passed a null pointer (see below)
**
**  Notes: 1. Two versions are supplied, one case-sensitive and one not.
**         2. Each version consists of a recursive static function which
**            does all the work, and a wrapper which checks for null
**            pointers before calling the static function.
*/
/*
**  Case-sensitive version
*/
static int patmat(const char *pat,const char *str)
{
	switch(*pat) {
	  case '\0': return !*str;
	  case '*' : return patmat(pat+1,str) || (*str && patmat(pat,str+1));
	  case '?' : return *str && patmat(pat+1,str+1);
	  default  : return (*pat==*str) && patmat(pat+1,str+1);
	}
}

static int xstrcmp(const char *pat,const char *str)
{
	if(NULL==str || NULL==pat) return -1;
	else                       return(patmat(pat, str));
}

#define stlWin2Linux(a) (a.convert('\\','/'))
#define stlLinux2Win(a) (a.convert('/','\\'))

//  /user/benno/test.txt -> test.txt
static STP getfile(const char *path)
{
	ansStl::cST fn = path;
#ifdef __linux__
	stlWin2Linux(fn);
	return fn.getDlm(-2,'/').getStp();
#else
	stlLinux2Win(fn);
	return fn.getDlm(-2,'\\').getStp();
#endif
}

//  /user/benno/test.txt -> /usr/benno/
static STP getdir(const char *path)
{
	ansStl::cST fn = path;
#ifdef __linux__
	stlWin2Linux(fn);
	fn.setDlm(-2,'/',"");
#else
	stlLinux2Win(fn);
	fn.setDlm(-2,'\\',"");
#endif
	return fn.getStp();
}


#if defined(__linux__) || defined(__APPLE__)

int stlFindFirst(const char *path,int flags,struct stlFind_t *buf)
{
	//printf("%s\n",path);
	memset(buf,0,sizeof(struct stlFind_t));
	buf->Flgs=flags;
	buf->pad=getdir(path);
	buf->fzk=getfile(path);

	//printf("pad='%s'",buf->pad ? buf->pad->sBuf : "NULL");
	//printf("fzk='%s'",buf->fzk ? buf->fzk->sBuf : "NULL");

	buf->pDIR=opendir(buf->pad->sBuf);
	if(buf->pDIR==NULL) {
		stlFree(buf->pad); buf->pad=NULL;
		stlFree(buf->fzk); buf->fzk=NULL;
		return -1;
	}
	return stlFindNext(buf);
}

int stlFindNext(struct stlFind_t *buf)
{
	struct dirent *de;
	struct stat st;
	STP tmp;

	//printf("stlFindNext %s",buf->pDIR ? "OK" : "NULL");

	if(buf->pDIR==NULL) {
		stlFree(buf->pad); buf->pad=NULL;
		stlFree(buf->fzk); buf->fzk=NULL;
		return -1;
	}

	while((de=readdir(buf->pDIR))!=NULL) {
		if(xstrcmp(buf->fzk->sBuf,de->d_name)==0) continue;

		tmp=stlCopy(buf->pad);
		stlAppendSt(tmp,de->d_name);
		stat(tmp->sBuf,&st);
		stlFree(tmp);

		//if (st.st_mode & S_IFDIR) printf("%02X %s\n",st.st_mode,de->d_name);
		//if (((st.st_mode & 0x0F)==0)&&(st.st_mode & S_IFDIR)) continue;
		if(st.st_mode & S_IFDIR){
			if((buf->Flgs & _stlFindDirs)==0) continue;
		}
		else {
			if((buf->Flgs & _stlFindNorm)==0) continue;
		}
		strcpy(buf->name,de->d_name);
		buf->size=st.st_size;
		return 0;
	}
	stlFindClose(buf);
	return -1;
}

int stlFindClose(struct stlFind_t *buf)
{
	if(buf->pDIR==NULL) return 0;
	closedir(buf->pDIR);
	buf->pDIR=NULL;
	stlFree(buf->pad); buf->pad=NULL;
	stlFree(buf->fzk); buf->fzk=NULL;
	return 0;
}

#endif

#ifdef _WIN32

static int stlFindNext_I(struct stlFind_t *buf)
{
	int iRes=0;
	while (1){
		int iNoBrk=0;
		int iBreak=0;
		if (buf->Flgs & _stlFindDirs){
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) iBreak++;
		}else{
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) iNoBrk++;
			else iBreak++;	// Hidden of gewoon bestand
		}
		if (buf->Flgs & _stlFindHid){
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) iBreak++;
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) iBreak++;
		}else{
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) iNoBrk++;
			if (buf->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) iNoBrk++;
		}
		if (iNoBrk) iBreak=0;
		if (iBreak) break;
		if (FindNextFile(buf->hFind, &buf->FindFileData)==0) return -1;
	}
	strncpy(buf->name,buf->FindFileData.cFileName,sizeof(buf->name)-1);
	buf->size=buf->FindFileData.nFileSizeLow;
	return 0;
}
int stlFindFirst(const char *path,int flags,struct stlFind_t *buf)
{
	memset(buf,0,sizeof(struct stlFind_t));
	buf->Flgs=flags;
	buf->hFind = FindFirstFile(path,&buf->FindFileData);
	if (buf->hFind == INVALID_HANDLE_VALUE) return -1;
	return stlFindNext_I(buf);
}

int stlFindNext (struct stlFind_t *buf)
{
	if (FindNextFile(buf->hFind, &buf->FindFileData)==0) return -1;
	return stlFindNext_I(buf);
}
int stlFindClose(struct stlFind_t *buf)
{
	if (buf->hFind == NULL) return 0;
	FindClose(buf->hFind);
	buf->hFind = NULL;
	return 0;
}

#endif

static bool my_sortTest(const ansStl::cST &v1,const ansStl::cST &v2) 
{
	return ((ansStl::cST &)v1) < v2;
}

std::vector<ansStl::cST> stlGetFilesAr(const char *zkp)
{
	std::vector<ansStl::cST> files;

	struct stlFind_t AF;
	int res = stlFindFirst(zkp, _stlFindNorm, &AF);
	while (res == 0) {
		files.push_back(AF.name);
		res = stlFindNext(&AF);
	}
	stlFindClose(&AF);

	std::sort(files.begin(), files.end(),my_sortTest);
	return files;
}
std::vector<ansStl::cST> stlGetFilesDirAr(const char *zkp)
{
	ansStl::cST dir = zkp;
	dir.convert('\\', '/');
	dir.setDlm(-2, '/', "");

	std::vector<ansStl::cST> files;

	struct stlFind_t AF;
	int res = stlFindFirst(zkp, _stlFindNorm, &AF);
	while (res == 0) {
		files.push_back(dir + AF.name);
		res = stlFindNext(&AF);
	}
	stlFindClose(&AF);

	std::sort(files.begin(), files.end(),my_sortTest);
	return files;
}

// Get al files in specific dir (sorted)
STP stlGetFiles(const char *zkp)
{
	std::vector<ansStl::cST> files = stlGetFilesAr(zkp);

	STP aRes = stlSetSt("");
	for (int i = 0; i < (int)files.size(); i++)
	{
		if (i) stlAppendCh(aRes, _D1);
		STP f = files[i].getStp();
		stlAppendStp(aRes, f);
		stlFree(f);
	}
	return aRes;
/*
	int iVld;
	int res;
	struct stlFind_t AF;
	STP aRes=stlSetSt("");
	res=stlFindFirst(zkp,_stlFindNorm,&AF);
	while (res==0){
		stlLocate(AF.name,aRes,0,0,&iVld,"I");
		//printf("add %d '%s'",iVld,AF.name);
		stlInsertStr(AF.name,aRes,iVld,0,0);
		res=stlFindNext(&AF);
	}
	stlFindClose(&AF);
	stlStripChr(aRes,_D1);
	if (aRes->iLen) return aRes;
	stlFree(aRes);
	return NULL;
*/
}

// get al files in specific DIR including path (sorted)
STP stlGetFilesDir(const char *zkp)
{
	std::vector<ansStl::cST> files = stlGetFilesDirAr(zkp);

	STP aRes = stlSetSt("");
	for (int i = 0; i < (int)files.size(); i++)
	{
		if (i) stlAppendCh(aRes, _D1);
		STP f = files[i].getStp();
		stlAppendStp(aRes,f);
		stlFree(f);
	}
	return aRes;
}
