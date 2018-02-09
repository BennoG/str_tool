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
#include "stl_str.h"
#include "stl_thread.h"

static char _stlExeName[300]="";

static void _stlInitName(void);

const char *stlIniName()
{
	_stlInitName();
	return _stlExeName;
}
void stlIniNameSet(const char *name)
{
	memset(_stlExeName,0,sizeof(_stlExeName));
	strncpy(_stlExeName,name,sizeof(_stlExeName)-5);
}

#ifdef __linux__
static void _stlInitName(void)
{
	STP sInf,sNam,sExe;
	if (_stlExeName[0]) return;     // name of this program already known
	sInf=stlFileRead("/proc/self/cmdline",0,-1);
	stlConvert(sInf,0,_D1);
	//stlConvert(sInf,'/',_D2);
	sNam=stlGetStr(sInf,1,0,0);		// complete naam inclusief path
	stlFree(sInf);

	// kijk eerst of we ook in etc dir een ini bestand hebben staan
	sExe = stlGetDlm(sNam,-2,'/');
	sInf = stlSetStf("/etc/%s.ini",sExe->sBuf);
	printf("ini etc test '%s' '%s'",sExe->sBuf,sInf->sBuf);
	stlFree(sExe);
	if (stlFileExist(sInf->sBuf))
	{
		strncpy(_stlExeName,sInf->sBuf,sizeof(_stlExeName)-4);
		stlFree(sNam);
		stlFree(sInf);
		printf("ini name=%s\n",_stlExeName);
		return;
	}
	stlFree(sInf);

	if ((sNam)&&(sNam->iLen))
		strncpy(_stlExeName,sNam->sBuf,sizeof(_stlExeName)-8);
	else
		strcpy(_stlExeName,"unknown");
	strcat(_stlExeName,".ini");
	printf("ini name=%s\n",_stlExeName);
}


/* Get info from .ini file
** sSection main part of ini file
** sItem    (optional) subitem in section of ini
**          when empty or NULL the whole section is returned
*/
STP stlIniGet(const char *sSection,const char *sItem)
{
	int iAct=0,iAdd,iIdx=1;
	STP sLin=NULL,sRes,sTmp;
	FILE *f1;
	_stlInitName();
	f1=fopen(_stlExeName,"rb");
	sRes=stlSetSt("");
	while (f1){
		if (sLin) stlFree(sLin);
		sLin=stlFileReadLine(f1);
		if (sLin==NULL) break;
		stlConvert(sLin,9,' ');
		if (sLin->sBuf[0]=='[')
		{
			stlConvert(sLin,'[',_D1);
			stlConvert(sLin,']',_D1);
			sTmp=stlGetStr(sLin,2,0,0);
			if (stricmp(sTmp->sBuf,sSection)==0) iAct=1; else iAct=0;
			stlFree(sTmp);
			continue;
		}
		if (iAct)
		{
			// remove coment data
			sTmp=stlGetDlm(sLin,1,';'); stlFree(sLin); sLin=sTmp;	
		
			sTmp=stlGetDlm(sLin,1,'='); stlStrip(sTmp);
			if (sTmp->iLen==0) {stlFree(sTmp); continue;}
			iAdd=0;
			if ((sItem)&&(sItem[0])&&(stricmp(sItem,sTmp->sBuf)==0)) iAdd=1;
			if ((sItem==NULL)||(sItem[0]==0)) iAdd=1;
			if (iAdd)
			{
				stlStoreStr(sTmp->sBuf,sRes,1,iIdx,0);
				stlFree(sTmp);
				sTmp=stlGetDlm(sLin,2,'='); stlStrip(sTmp);
				stlStoreStr(sTmp->sBuf,sRes,2,iIdx,0);
				iIdx++;
			}
			stlFree(sTmp);
		}
	}
	if (f1) fclose(f1);
	if (iIdx==1){stlFree(sRes); return NULL;}
	if ((sItem)&&(sItem[0])){sTmp=stlGetStr(sRes,2,0,0); stlFree(sRes); sRes=sTmp;stlConvert(sRes,_D2,_D1);}
	return sRes;
}

/* Set info into .ini file
**  sSection main part of ini file
**  sItem    subitem in section of ini
**  sValue   new value for item (NULL remove item)
** Return 0 Ok
**       -1 Section part is invalid
**       -2 item part is invalid
*/
int stlIniSet(const char *sSection,const char *sItem,const char *sValue)
{
	static stlMutex mux=stlMutexInitializer;
	int iSect=-1;	// start of section
	int i,nri,iAct=0,iDone=0;
	STP sFil,sLin,sTmp;
	if ((sSection==NULL)||(sSection[0]==0)) return -1;	// section is required on update
	if ((sItem==NULL)||(sItem[0]==0)) return -2;	// item is also required on update
	_stlInitName();
	stlMutexLock(&mux);
	sFil=stlFileRead(_stlExeName,0,-1);
	if (sFil==NULL){stlMsWait(100); sFil=stlFileRead(_stlExeName,0,-1);}
	if (sFil==NULL) sFil=stlSetSt("");
	stlRemove(sFil,13);
	nri=stlConvert(sFil,10,_D1)+1;	// lines
	stlConvert(sFil,';',_D2);		// comment
	stlConvert(sFil,'=',_D3);		// value / item separator
	stlConvert(sFil, 9 , 32);		// TAB's to spaces
	for (i=1;i<=nri;i++)
	{
		sLin=stlGetStr(sFil,i,0,0);
		if (sLin->sBuf[0]=='[')
		{
			stlConvert(sLin,'[',_D1);
			stlConvert(sLin,']',_D1);
			sTmp=stlGetStr(sLin,2,0,0);
			if (stricmp(sTmp->sBuf,sSection)==0) iAct=1; else iAct=0;
			stlFree(sTmp);
			if ((iAct)&&(iSect<0)) iSect=i;		// insert position if not found
			stlFree(sLin);
			continue;
		}
		if (iAct)
		{
			sTmp=stlGetStr(sFil,i,1,1);			// item tag
			stlStrip(sTmp);
			if (stricmp(sTmp->sBuf,sItem)==0)	// found we could update the data
			{
				if (sValue)
				{
					stlStoreStr(sValue,sFil,i,1,2);	// update value in data
				}else
				{
					stlDelStr(sFil,i,0,0);		// remove value from data
					i--;
				}
				iDone++;
			}
			stlFree(sTmp);
		}
		stlFree(sLin);
	}
	if ((iDone==0)&&(sValue))		// not found add to section
	{
		if (iSect<0)				// No section create section
		{
			stlAppendStf(sFil,"\n[%s]\n%s=%s\n",sSection,sItem,sValue);
		}else
		{
			stlInsertStrf(sFil,iSect+1,0,0,"%s=%s",sItem,sValue);
		}
		iDone++;
	}
	if (iDone)
	{
		stlConvert(sFil,_D1,10 );	// lines
		stlConvert(sFil,_D2,';');	// comment
		stlConvert(sFil,_D3,'=');	// value / item separator
		stlFileWrite(sFil,_stlExeName);
	}
	stlMutexUnlock(&mux);
	return 0;
}
#endif

#ifdef _WIN32

static void _stlInitName()
{
	STP sNam,sDbg;

	if (_stlExeName[0]) return;     // name of this program already known

	sNam=stlInitLen(300,0);
	GetModuleFileName(NULL,sNam->sBuf,sNam->iLen-1);
	sNam->iLen=strlen(sNam->sBuf);
	// Remove debug or release part from full filename (for development options)
	sDbg=stlGetDlm(sNam,-3,'\\');
	if (stricmp(sDbg->sBuf,"debug"  )==0) stlDelDlm(sNam,-3,'\\');
	if (stricmp(sDbg->sBuf,"release")==0) stlDelDlm(sNam,-3,'\\');
	stlFree(sDbg);

	stlDelDlm(sNam,-2,'.');
	if ((sNam)&&(sNam->iLen)) 
		strncpy(_stlExeName,sNam->sBuf,sizeof(_stlExeName)-8);
	else
		strcpy(_stlExeName,"unknown");
	strcat(_stlExeName,".ini");
	stlFree(sNam);
}

/* Get info from .ini file
** sSection main part of ini file
** sItem    (optional) subitem in section of ini
**          when empty or NULL the whole section is returned
*/
STP stlIniGet(const char *sSection,const char *sItem)
{
	int iLen,i,nri;
	STP sRes=NULL,sSec,sNam;
	_stlInitName();
	if ((sItem)&&(sItem[0]))
	{
		sRes=stlSetSt("");
		sSec=stlInitLen(10240,0);
		iLen=GetPrivateProfileSection(sSection,sSec->sBuf,sSec->iLen,_stlExeName);
		sSec->iLen = iLen;
		nri=stlConvert(sSec, 0 ,_D1);
		stlConvert(sSec,'=',_D2);
		stlConvert(sSec,';',_D3);
		stlConvert(sSec, 9 ,' ');	// Remove TAB's

		for (i=0;i<=nri;i++)
		{
			sNam = stlGetStr(sSec,i+1,1,0); stlStrip(sNam);
			if (stricmp(sNam->sBuf,sItem)==0)
			{
				stlFree(sNam);
				sNam = stlGetStr(sSec,i+1,2,1);
				if (sRes->iLen) stlAppendCh(sRes,_D1);
				stlStrip(sNam);					// Remove leading and trailing spaces
				stlAppendStp(sRes,sNam);
			}
			stlFree(sNam);
		}
		stlFree(sSec);
	}else
	{
		sRes=stlInitLen(10240,0);
		iLen=GetPrivateProfileSection(sSection,sRes->sBuf,sRes->iLen,_stlExeName);
		if (iLen>=0) sRes->iLen=iLen;
		stlConvert(sRes, 0 ,_D1);
		stlConvert(sRes,'=',_D2);
		stlRotate(sRes);
	}
	return sRes;
}

/* Set info into .ini file
**  sSection main part of ini file
**  sItem    subitem in section of ini
**  sValue   new value for item (NULL remove item)
** Return 0 Ok
**       -1 Section part is invalid
**       -2 item part is invalid
*/
int stlIniSet(const char *sSection,const char *sItem,const char *sValue)
{
	if ((sSection==NULL)||(sSection[0]==0)) return -1;	// section is required on update
	if ((sItem==NULL)||(sItem[0]==0)) return -2;	// item is also required on update
	_stlInitName();
	WritePrivateProfileString(sSection,sItem,sValue,_stlExeName);
	return 0;
}


#endif

/* Get info from .ini file
** sSection  main part of ini file
** sItem     subitem in section of ini
** sDefault  Default value if not in .ini file or empty
*/
STP stlIniGetD(const char *sSection,const char *sItem,const char *sDefault)
{
	STP sRes;
	sRes=stlIniGet(sSection,sItem);
	if ((sRes)&&(sRes->iLen==0)){stlFree(sRes);sRes=NULL;}
	if (sRes==NULL) sRes=stlSetSt(sDefault);
	return sRes;
}

int stlIniGetIntD(const char *sSection,const char *sItem,int iDefault)
{
	STP sRes = stlIniGet(sSection,sItem);
	if ((sRes)&&(sRes->iLen > 0)) iDefault = strtol(sRes->sBuf,NULL,0);
	stlFree(sRes);
	return iDefault;
}
