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
#include "stl_xml.h"
#ifdef __linux__
#  include <unistd.h>
#endif
#include <stdlib.h>

#ifdef _WIN32
#  ifdef _WIN64
#     pragma comment(lib,"libxml2.dll.a")
#  else
#     pragma comment(lib,"libxml2.lib")
#  endif // _M_X64
#endif

#ifdef __linux__
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#endif
#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
#endif


stlXml::stlXml(xmlDocPtr doc,const char* fileName /* = NULL */)
{
	initVars();
	stlMutexLock(&docMux);
	if (fileName){
		STP tmp = stlSetSt(fileName);
		if (sFileName) 
			stlExchange(sFileName,tmp);
		else { sFileName = tmp; tmp = NULL; }
		stlFree(tmp);
	}
	cfgDoc = doc;
	docRelease = false;
	stlMutexUnlock(&docMux);
}

stlXml::stlXml(const char* fileName)
{
	initVars();
	loadXml(fileName);
}
stlXml::stlXml(STP sXmlData)
{
	initVars();
	parseSTP(sXmlData);
}
stlXml::~stlXml()
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	_uid = 0;
	stlMutexLock(&docMux);
	if ((docRelease) && (cfgDoc)) xmlFreeDoc(cfgDoc);
	cfgDoc = NULL;
	stlFree(sFileName);
	sFileName = NULL;
	stlMutexUnlock(&docMux);
	stlMutexRelease(&docMux);
}

void stlXml::parseSTP(STP sXmlData)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	stlMutexLock(&docMux);
	if ((docRelease) && (cfgDoc)){ xmlFreeDoc(cfgDoc); cfgDoc = NULL; }
	cfgDoc = xmlParseMemory(sXmlData->sBuf,sXmlData->iLen);
	if (cfgDoc == NULL) printf("stlXml::parseSTP Error parsing xml data");
	stlMutexUnlock(&docMux);
}

void stlXml::loadXml(const char *fileName /* = NULL */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	bool bReadErr = false;
	stlMutexLock(&docMux);
	//xmlParserCtxtPtr ctxt = NULL;
	//ctxt = xmlCreateFileParserCtxt(fileName);
	//xmlFreeParserCtxt(ctxt);
	//ctxt = xmlCreateDocParserCtxt("UTF-8");
	//xmlFreeParserCtxt(ctxt);
	if ((docRelease) && (cfgDoc)) xmlFreeDoc(cfgDoc);
	cfgDoc = NULL;
	if ((fileName == NULL) && (sFileName))
	{
		cfgDoc = xmlParseFile(sFileName->sBuf);
		fileName = sFileName->sBuf;
	}else
	{
		if (sFileName) {stlFree(sFileName); sFileName = NULL; }
		if (fileName){
			cfgDoc = xmlParseFile(fileName);
			sFileName = stlSetSt(fileName);
		}
	}
	if (cfgDoc == NULL){
		bReadErr = true;
		STP sFn2 = stlSetStf("%s.old",fileName);
		cfgDoc = xmlParseFile(sFn2->sBuf);	// Backup van vorige opslaan actie
		stlFree(sFn2);
	}
	if (cfgDoc == NULL)
	{
		bReadErr = true;
		STP sFn2 = stlSetStf("%s.ok",fileName);
		cfgDoc = xmlParseFile(sFn2->sBuf);		// laatste vangnet
		stlFree(sFn2);
	}
	if ((bReadErr) && (cfgDoc))					// main document was onleesbaar dus nu opslaan
	{
		unlink(fileName);
		xmlSaveFormatFile(fileName,cfgDoc,1);
	}
	if (cfgDoc)
	{
		STP sFn2 = stlSetStf("%s.ok",fileName);
		xmlSaveFormatFile(sFn2->sBuf,cfgDoc,1);	// laatste vangnet
		stlFree(sFn2);
	}
	stlMutexUnlock(&docMux);
#ifdef __linux__
	int iRes = system("sync");
	if (iRes) printf("System sync error");
#endif
}


int stlXml::saveDocument(const char *fn /* = NULL */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	stlMutexLock(&docMux);
	if ((fn)&&(fn[0]))
	{
		unlink(fn);
		xmlSaveFormatFile(fn,cfgDoc,1);
	}else if (sFileName)
	{
		STP sFn2 = stlSetStf("%s.old",sFileName->sBuf);
		unlink(sFn2->sBuf);
		rename(sFileName->sBuf , sFn2->sBuf);
		xmlSaveFormatFile(sFileName->sBuf,cfgDoc,1);
		stlFree(sFn2);
	}
	stlMutexUnlock(&docMux);
#ifdef __linux__
	int iRes = system("sync");
	if (iRes) printf("System sync error");
#endif
	return 0;
}

STP stlXml::getDocMem(int iFormat /* = 0 */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	xmlChar *data = NULL;
	int dataLen = 0;
	stlMutexLock(&docMux);
	xmlDocDumpFormatMemory(cfgDoc,&data,&dataLen,iFormat);
	stlMutexUnlock(&docMux);
	STP sXml = stlInitLenSrc(dataLen,(char*)data);
	xmlFree(data);
	return sXml;
}


STP stlXml::cfgGetXpathXML(const char *sXpath)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int i;
	STP sRes = NULL, sNode = NULL;
	xmlNodePtr         xNod;
	xmlXPathContextPtr xCfg =NULL; 
	xmlXPathObjectPtr  xObjs=NULL;
	if (cfgDoc == NULL) return NULL;
	stlMutexLock(&docMux);
	xCfg=xmlXPathNewContext(cfgDoc);
	if (xCfg==NULL){printf("error preparing xpath\n");stlMutexUnlock(&docMux);return NULL;}
	xObjs=xmlXPathEval((xmlChar*)sXpath,xCfg);
	if ((xObjs)&&(xObjs->nodesetval)&&(xObjs->nodesetval->nodeNr)){
		for (i=0;i<xObjs->nodesetval->nodeNr;i++){
			xmlBufferPtr bufferPtr = xmlBufferCreate();
			xNod=xmlXPathNodeSetItem(xObjs->nodesetval,i);
			xmlNodeDump(bufferPtr,NULL,xNod,0,1);
			sNode = stlInitLenSrc(bufferPtr->use,(char*)bufferPtr->content);
			if (sRes){
				stlAppendStp(sRes,sNode);
				stlFree(sNode);
			}else sRes = sNode;
			xmlBufferFree(bufferPtr);
		}
	}
	if (xObjs) xmlXPathFreeObject(xObjs);// Release allocated memory
	if (xCfg)  xmlXPathFreeContext(xCfg);
	stlMutexUnlock(&docMux);
	return sRes;
}

STP stlXml::cfgGetXpathAXML(const char *pStr,va_list args_lst)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP sXpath,sRes;
	sXpath=stlSetSta(pStr,args_lst);
	sRes=cfgGetXpathXML(sXpath->sBuf);
	stlFree(sXpath);
	return sRes;
}
STP stlXml::cfgGetXpathXMLf(const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	STP sRes = cfgGetXpathAXML(fmt,ap);
	va_end(ap);
	return sRes;
}

int stlXml::cfgDelXpath(const char *sXpath)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int i,iRes=0;
	xmlNodePtr         xNod;
	xmlXPathContextPtr xCfg =NULL; 
	xmlXPathObjectPtr  xObjs=NULL;
	if (cfgDoc == NULL) return -1;
	stlMutexLock(&docMux);
	xCfg=xmlXPathNewContext(cfgDoc);
	if (xCfg==NULL){printf("error preparing xpath\n");stlMutexUnlock(&docMux);return -2;}

	xObjs=xmlXPathEval((xmlChar*)sXpath,xCfg);
	if ((xObjs)&&(xObjs->nodesetval)&&(xObjs->nodesetval->nodeNr)){
		for (i=0;i<xObjs->nodesetval->nodeNr;i++){
			xNod=xmlXPathNodeSetItem(xObjs->nodesetval,i);
			xmlUnlinkNode(xNod);
			xmlFreeNode(xNod);
			updateCount++;
		}
	}
	if (xObjs) xmlXPathFreeObject(xObjs);// Release allocated memory
	if (xCfg)  xmlXPathFreeContext(xCfg);
	stlMutexUnlock(&docMux);
	return iRes;

}
int stlXml::cfgDelXpathA(const char *pStr,va_list args_lst)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP sXpath = stlSetSta(pStr,args_lst);
	int iRes = cfgDelXpath(sXpath->sBuf);
	stlFree(sXpath);
	return iRes;
}
int stlXml::cfgDelXpathf(const char *sFmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,sFmt);
	int iRes = cfgDelXpathA(sFmt,ap);
	va_end(ap);
	return iRes;
}



/* get xpath string(s) van configuratie
*	/config/machine/grab/init/cam[@id='0']/frame_offset  (geeft camera offset camera ID=0)
*	/config/machine/grab/init/camera/num_lines
*/
STP stlXml::cfgGetXpath(const char *sXpath)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int i;
	STP sRes=NULL;
	xmlChar           *xCont;
	xmlNodePtr         xNod;
	xmlXPathContextPtr xCfg =NULL; 
	xmlXPathObjectPtr  xObjs=NULL;
	if (cfgDoc == NULL) return NULL;
	stlMutexLock(&docMux);
	xCfg=xmlXPathNewContext(cfgDoc);
	xObjs=xmlXPathEval((xmlChar*)sXpath,xCfg);
	if ((xObjs)&&(xObjs->nodesetval)&&(xObjs->nodesetval->nodeNr)){
		for (i=0;i<xObjs->nodesetval->nodeNr;i++){
			xNod=xmlXPathNodeSetItem(xObjs->nodesetval,i);
			xCont=xmlNodeGetContent(xNod);
			if (sRes) stlAppendStf(sRes,"%c%s",_D1,xCont);
			else      sRes=stlSetSt((char*)xCont);
			xmlFree(xCont);
		}
	}
	if (xObjs) xmlXPathFreeObject(xObjs);// Release allocated memory
	if (xCfg)  xmlXPathFreeContext(xCfg);
	stlMutexUnlock(&docMux);
	return sRes;
}

stlXmlList *stlXml::cfgGetXpathListf(const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	STP sXpath = stlSetSta(fmt,ap);
	va_end(ap);
	xmlXPathContextPtr xCfg = xmlXPathNewContext(cfgDoc);
	stlXmlList *lRes = new stlXmlList(xmlXPathEval((xmlChar*)sXpath->sBuf,xCfg));
	if (xCfg)  xmlXPathFreeContext(xCfg);
	stlFree(sXpath);
	return lRes;
}

STP stlXml::cfgGetXpathA(const char *pStr,va_list args_lst)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP sXpath,sRes;
	sXpath=stlSetSta(pStr,args_lst);
	sRes=cfgGetXpath(sXpath->sBuf);
	stlFree(sXpath);
	return sRes;
}

STP stlXml::cfgGetXpathf(const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	STP sRes = cfgGetXpathA(fmt,ap);
	va_end(ap);
	return sRes;
}

int stlXml::cfgGetXpathIntDf(int iDefault,const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	STP sRes = cfgGetXpathA(fmt,ap);
	va_end(ap);
	if ((sRes)&&(sRes->iLen))
	{
		if ((sRes->iLen>1)&&((sRes->sBuf[1]=='x')||(sRes->sBuf[1]=='X')))
			iDefault = strtoul(sRes->sBuf,NULL,0);
		else
			iDefault = strtol(sRes->sBuf,NULL,0);
	}
	stlFree(sRes);
	return iDefault;
}
double stlXml::cfgGetXpathDoubleDf(double dDefault,const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,fmt);
	STP sRes = cfgGetXpathA(fmt,ap);
	va_end(ap);
	if ((sRes)&&(sRes->iLen))
		dDefault = strtod(sRes->sBuf,NULL);
	stlFree(sRes);
	return dDefault;
}

STP stlXml::getModuleParameter(const char* module,const char* parName)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return cfgGetXpathf("/config/machine/modules/module[@id='%s']/param[@id='%s']",module,parName);
}
STP stlXml::getModuleParameter(const char* module,const char* parName,const char* sDefault)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP res = getModuleParameter(module,parName);
	if ((res == NULL) && (sDefault)) res = stlSetSt(sDefault);
	return res;
}
STP stlXml::getModuleParameterf(const char* module,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP sPar = stlSetSta(parName,ap);
	va_end(ap);
	STP sRes = getModuleParameter(module,sPar->sBuf);
	stlFree(sPar);
	return sRes;
}

STP stlXml::getModuleParamAttrib(const char *sModule,const char *attrib,const char *parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP sPar = stlSetSta(parName,ap);
	va_end(ap);
	STP sRes = cfgGetXpathf("/config/machine/modules/module[@id='%s']/param[@id='%s']/@%s",sModule,sPar->sBuf,attrib);
	stlFree(sPar);
	return sRes;
}
int stlXml::getModuleParameterInt(const char* module,const char* parName)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return cfgGetXpathIntDf(0,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,parName);
}
int stlXml::getModuleParameterInt(const char* module,const char* parName,int iDefault)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return cfgGetXpathIntDf(iDefault,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,parName);
}
double stlXml::getModuleParameterDbl(const char* module,const char* parName)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return getModuleParameterDbl(module,parName,0.0);
}
double stlXml::getModuleParameterDbl(const char* module,const char* parName,const double dDefault)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	double dRes = dDefault;
	STP st = getModuleParameter(module,parName);
	if ((st) && (st->iLen))
		dRes = stlGetDlmDouble(st,0,0);
	stlFree(st);
	return dRes;
}
int stlXml::setModuleParameter(const double dVal,const char* module,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	STP va = stlSetStf("%f",dVal);
	int iRes = cfgSetXpathf(va->sBuf,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,xp->sBuf);
	stlFree(xp);
	stlFree(va);
	return iRes;
}
int stlXml::setModuleParameter(const int iVal,const char* module,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	STP va = stlSetStf("%d",iVal);
	int iRes = cfgSetXpathf(va->sBuf,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,xp->sBuf);
	stlFree(xp);
	stlFree(va);
	return iRes;
}

int stlXml::setModuleParAttrib(const char *attValue,const char* module, const char *attName, const char* parName, ...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap, parName);
	STP xp = stlSetSta(parName, ap);
	va_end(ap);
	int iRes = cfgSetXpathf(attValue,"/config/machine/modules/module[@id='%s']/param[@id='%s']/@%s", module, xp->sBuf, attName);
	stlFree(xp);
	return iRes;
}


int stlXml::setModuleParameter(const char *sVal,const char* module,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	int iRes = cfgSetXpathf(sVal,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,xp->sBuf);
	stlFree(xp);
	return iRes;
}
int stlXml::setModuleParameter(const ansStl::cST &sVal,const char* module,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	int iRes = cfgSetXpathf(sVal,"/config/machine/modules/module[@id='%s']/param[@id='%s']",module,xp->sBuf);
	stlFree(xp);
	return iRes;
}

int stlXml::setCANDeviceParameter(int canBus,int canID,const char* parName,const char *value)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	if ((parName == NULL) || (parName[0] == 0)) return -2;
	if (value == NULL)		// delete als waarde NULL is
		return cfgDelXpathf("/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']",canBus,canID,parName);
	return cfgSetXpathf(value,"/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']",canBus,canID,parName);
}

int stlXml::setCANDeviceParameter(int canBus,int canID,const char* parName,int value)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	if ((parName == NULL) || (parName[0] == 0)) return -2;
	ansStl::cST val;
	val.setf("%d",value);
	return cfgSetXpathf(val,"/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']",canBus,canID,parName);
}

int stlXml::setCANDeviceParameter(int canBus,int canID,const char* parName,double value)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	if ((parName == NULL) || (parName[0] == 0)) return -2;
	ansStl::cST val;
	val.setf("%f",value);
	return cfgSetXpathf(val,"/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']",canBus,canID,parName);
}
int stlXml::setCANDeviceParFactor(int canBus,int canID,const char* parName,int iFactor)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	if ((parName == NULL) || (parName[0] == 0)) return -2;
	ansStl::cST val;
	val.setf("%d",iFactor);
	return cfgSetXpathf(val,"/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']/@fact",canBus,canID,parName);
}
int stlXml::setCANDeviceParAttrib(int canBus,int canID,const char* parName,const char *attName,const char *fmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	if ((parName == NULL) || (parName[0] == 0) || (attName == NULL) || (attName[0] == 0) || (fmt == NULL)) return -2;
	va_list ap;
	va_start(ap,fmt);
	ansStl::cST val(ap,fmt);
	va_end(ap);
	return cfgSetXpathf(val,"/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']/@%s",canBus,canID,parName,attName);
}

STP stlXml::getCANDeviceParameter(int canBus,int canID,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	STP sRes = cfgGetXpathf("/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']",canBus,canID,xp->sBuf);
	stlFree(xp);
	return sRes;
}
int stlXml::getCANDeviceParFactor(int canBus,int canID,const char* parName,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int iRes = 1;
	va_list ap;
	va_start(ap,parName);
	STP xp = stlSetSta(parName,ap);
	va_end(ap);
	STP sRes = cfgGetXpathf("/config/can/devices/device/bus[text()='%d']/../address[text()='%d']/../param[@id='%s']/@fact",canBus,canID,xp->sBuf);
	if ((sRes) && (sRes->iLen)) iRes = stlGetDlmIntAuto(sRes,0,0);
	stlFree(sRes);
	stlFree(xp  );
	return iRes;
}


STP stlXml::getConfigParameter(const char* parName,const char *sDefault /* = NULL */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP res = cfgGetXpathf("/config/%s",parName);
	if ((res == NULL) && (sDefault)) res = stlSetSt(sDefault);
	return res;
}
int stlXml::getConfigParameterInt(const char* parName,const int iDefault /* = 0 */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return cfgGetXpathIntDf(iDefault,"/config/%s",parName);
}
STP stlXml::getMachineConfigParameter(const char* parName,const char *sDefault /* = NULL */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	STP res = cfgGetXpathf("/config/machine/param[@id='%s']",parName);
	if ((res == NULL) && (sDefault)) res = stlSetSt(sDefault);
	return res;
}
int stlXml::getMachineConfigParameterInt(const char* parName,const int iDefault /* = 0 */)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	return cfgGetXpathIntDf(iDefault,"/config/machine/param[@id='%s']",parName);
}


/************************************************************************/
/* toevoegen aanpassen xml parameters                                   */
/************************************************************************/

/* Set xpath string van configuratie
*	/config/machine/grab/init/cam[@id='0']/frame_offset  (set camera offset camera ID=0)
*	/config/machine/grab/init/camera/num_lines
* Return 0 Ok
*       <0 Error
*/
int stlXml::cfgSetXpath(const char *sValue,const char *sXpath)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int iRes=-2;
	xmlXPathContextPtr xCfg =NULL; 
	if (cfgDoc == NULL) return -1;
	stlMutexLock(&docMux);
	xCfg=xmlXPathNewContext(cfgDoc);
	if (xCfg==NULL){printf("error preparing xpath\n");stlMutexUnlock(&docMux);return -1;}

	if (stlXmlAddUpdContent(xCfg,sXpath,(xmlChar*)sValue)==0){
		updateCount++;
		iRes=0;
	}
	if (xCfg)  xmlXPathFreeContext(xCfg);
	stlMutexUnlock(&docMux);
	return iRes;
}

int stlXml::cfgSetXpathf(const char *sValue,const char *sFmt,...)
{
	if (_uid != _StrXmlUidC_) _lxBreak_();
	int iRes;
	STP sXpath;
	va_list ap;

	va_start(ap,sFmt);
	sXpath=stlSetSta(sFmt,ap);
	va_end(ap);
	iRes=cfgSetXpath(sValue,sXpath->sBuf);
	stlFree(sXpath);
	return iRes;
}



/************************************************************************/
/*                                                                      */
/************************************************************************/

STP stlXmlList::getParVal(int iIdx,va_list args_lst,const char *fmt)
{
	STP sRes   = NULL;
	STP sXpath = stlSetSta(fmt,args_lst);
	if ((xpList)&&(xpList->nodesetval)&&(xpList->nodesetval->nodeNr) &&
		(iIdx>=0) && (iIdx < xpList->nodesetval->nodeNr))
	{
		xmlNodePtr nod = xmlXPathNodeSetItem(xpList->nodesetval, iIdx);
		const char *cp = stlXmlNodXPathStringf(nod,sXpath->sBuf); 
		if (cp) sRes = stlSetSt(cp);
		nod=NULL;
	}
	stlFree(sXpath);
	return sRes;
}
STP stlXmlList::getParVal(int iIdx,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP sRes = getParVal(iIdx,ap,fmt);
	va_end(ap);
	return sRes;
}

STP stlXmlList::getParName(int iIdx,va_list args_lst,const char *fmt)
{
	STP sRes   = NULL;
	STP sXpath = stlSetSta(fmt,args_lst);
	if ((xpList)&&(xpList->nodesetval)&&(xpList->nodesetval->nodeNr) &&
		(iIdx>=0) && (iIdx < xpList->nodesetval->nodeNr))
	{
		xmlNodePtr nod = xmlXPathNodeSetItem(xpList->nodesetval, iIdx);
		const char *cp = stlXmlNodXPathNamef(nod,sXpath->sBuf); 
		if (cp) sRes = stlSetSt(cp);
		//		if ((nod) && (nod->type == XML_ELEMENT_NODE) && (nod->name))
		//			sRes = stlSetSt((char*)nod->name);
		nod=NULL;
	}
	stlFree(sXpath);
	return sRes;
}
int stlXmlList::deleteItem(int iIdx)
{
	if ((xpList)&&(xpList->nodesetval)&&(xpList->nodesetval->nodeNr) &&
		(iIdx>=0) && (iIdx < xpList->nodesetval->nodeNr))
	{
		xmlNodePtr xNod = xmlXPathNodeSetItem(xpList->nodesetval, iIdx);
		xmlUnlinkNode(xNod);
		xmlFreeNode(xNod);
		return 0;
	}
	return -1;
}
STP stlXmlList::getParName(int iIdx,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP sRes = getParName(iIdx,ap,fmt);
	va_end(ap);
	return sRes;
}

stlXmlList * stlXmlList::getXPathList(int iIdx,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP sXpt = stlSetSta(fmt,ap);
	va_end(ap);
	stlXmlList *lRes = NULL;
	if ((xpList) && (xpList->nodesetval) && (xpList->nodesetval->nodeNr) &&
		(iIdx>=0) && (iIdx < xpList->nodesetval->nodeNr))
	{
		xmlNodePtr nod = xmlXPathNodeSetItem(xpList->nodesetval, iIdx);
		if (nod)
		{
			xmlXPathContextPtr xCfg = xmlXPathNewContext(nod->doc);
			if (xCfg) xCfg->node = nod;
			lRes = new stlXmlList(xmlXPathEval((xmlChar*)sXpt->sBuf,xCfg));
			if (xCfg)  xmlXPathFreeContext(xCfg);
		}
	}
	stlFree(sXpt);
	return lRes;
}



int stlXmlList::getParInt(int iIdx,int iDefault,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP sRes = getParVal(iIdx,ap,fmt);
	va_end(ap);
	if ((sRes) && (sRes->iLen)) iDefault = strtoul(sRes->sBuf,NULL,0);
	stlFree(sRes);
	return iDefault;
}
int stlXmlList::getParInt(int iIdx,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	STP sRes = getParVal(iIdx,ap,fmt);
	va_end(ap);
	int iDefault = 0;
	if ((sRes) && (sRes->iLen)) iDefault = strtoul(sRes->sBuf,NULL,0);
	stlFree(sRes);
	return iDefault;
}

