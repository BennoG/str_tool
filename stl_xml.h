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
#ifndef _STL_XML_H_
#define _STL_XML_H_

#ifdef _WIN32
#  include <libxml_win/xmlmemory.h>
#  include <libxml_win/parser.h>
#  include <libxml_win/xpath.h>
#  include <libxml_win/xpointer.h>
//#  include <libxml_win/parserInternals.h>
#endif

#ifdef __linux__
#  include <libxml2/libxml/xmlmemory.h>
#  include <libxml2/libxml/parser.h>
#  include <libxml2/libxml/xpath.h>
#  include <libxml2/libxml/xpointer.h>
//#  include <libxml2/parserInternals.h>
#endif

#include "stl_str.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// stlXmlNodeStrSelect   -> stlXmlNodXPathStringf
// stlXmlNodeStrSelectNN -> stlXmlNodXPathStringfNN

// geef node terug voor b.v. (deze ook niet meer gebruiken) hewoon xpath functies
// "weight"
// "weight(unitcode)"
// "unit/type"
// "unit/type(barcode)"
//xmlNodePtr stlXmlNodeSelect(xmlNodePtr cur,char *sPath);

/*
/config/can/devices/device/function[text()='encoder']/../address
/config/tulip/grab/init/cam[@id='1']
*/

/* Selecteer xpath values uit de object.xml file
 * formatering volgens printf formaat
 */
xmlXPathObjectPtr stlXmlXPathSelectf(xmlXPathContextPtr xptr,const char *fmt,...);
xmlXPathObjectPtr stlXmlNodXPathSelectf(xmlNodePtr xNod,const char *fmt,...);

/* Select xPath from (document / context / node )
** Return value of selected node's _D1 delimited
*/
STP stlXmlXPathValuef(xmlXPathContextPtr xptr,const char *fmt,...);
STP stlXmlNodXPathValuef(xmlNodePtr nod,const char *fmt,...);
STP stlXmlDocXPathValuef(xmlDocPtr doc,const char *fmt,...);

/* Geeft node terug van xpath selectie
 *	Return NULL als niet gevonden
 */
xmlNodePtr stlXmlXPathFirstNodef(xmlXPathContextPtr xptr,const char *fmt,...);
xmlNodePtr stlXmlDocXPathFirstNodef(xmlDocPtr doc,const char *fmt,...);

/* returns XML string pointer of relative xpath result data valid as long as document is valid
*/
const char* stlXmlNodXPathNamef  (xmlNodePtr nod,const char *fmt,...);
const char* stlXmlNodXPathStringf(xmlNodePtr nod,const char *fmt,...);
const char* stlXmlXPathStringf   (xmlXPathContextPtr xptr,const char *fmt,...);

/* returns XML string pointer of relative xpath result data valid as long as document is valid
** Return is never NULL ("" instead)
*/
const char* stlXmlNodXPathStringfNN(xmlNodePtr nod,const char *fmt,...);
const char* stlXmlXPathStringfNN   (xmlXPathContextPtr xptr,const char *fmt,...);

/* deze zijn vervallen gebruik stlXmlNodXPathStringf en stlXmlNodXPathStringfNN
// Zie amsXmlNodeSelect
// geeft echter de waarde terug i.p.v. de node (kan ook NULL terug geven als niet bestaat)
char *stlXmlNodeStrSelect(xmlNodePtr cur,char *sPath);

// Zie amsXmlNodeSelect
// geeft echter de waarde terug i.p.v. de node (geeft lege string terug als niet bestaat)
char *stlXmlNodeStrSelectNN(xmlNodePtr cur,char *sPath);

// kan meerdere antwoorden terug geven _FM delimited
STP stlXmlNodesSelect(xmlNodePtr cur,char *sPath,char *sNodeName);
*/

// Dump tree in STP
STP stlXmlNodeData(xmlNodePtr cur);

/* Add content to xml document (take notice on attributes)
 *  xDoc   document context
 *  sXpath xpath naar attribute
 *  xCont  value to store
 * Return 0 Ok
 *        1 Error
 */
int stlXmlAddUpdContent(xmlXPathContextPtr xDoc,const char *sXpath,xmlChar *xCont);

#ifdef __cplusplus
}


class stlXmlList
{
private:
	void initVars(){xpList = NULL; }
	xmlXPathObjectPtr xpList;
public:
	stlXmlList(xmlXPathObjectPtr l){initVars(); xpList = l; }
	~stlXmlList(){if (xpList) xmlXPathFreeObject(xpList); xpList = NULL;}
	int length(){return ((xpList) && (xpList->nodesetval) ? xpList->nodesetval->nodeNr : 0);}
	int count(){return length();}
	// get xpath parameter of selection "IP" for value of <IP>127.0.0.1</IP>
	//                                  "@id" for attribute value of id=""
	STP getParVal(int iIdx,const char *fmt,...);
	STP getParVal(int iIdx,va_list args_lst,const char *fmt);
	int getParInt(int iIdx,int iDefault,const char *fmt,...);
	int getParInt(int iIdx,const char *fmt,...);
	STP getParName(int iIdx,const char *fmt,...);
	STP getParName(int iIdx,va_list args_lst,const char *fmt);
	stlXmlList * getXPathList(int iIdx,const char *fmt,...);
	int deleteItem(int iIdx);
};

#define _StrXmlUidC_	0x5A1234A7

class stlXml
{
protected:
	bool docRelease;
	STP sFileName;
	xmlDocPtr cfgDoc;
	stlMutex  docMux;
	int _uid;
public:
	int updateCount;	// the number of modifications made to this xml
private:
	void initVars(){cfgDoc = NULL;stlMutexInit(&docMux); sFileName = NULL; updateCount = 0; docRelease = true; _uid = _StrXmlUidC_; }
public:
	stlXml(const char* fileName);
	stlXml(xmlDocPtr doc,const char* fileName = NULL);
	stlXml(STP sXmlData);
	~stlXml();
	xmlDocPtr getDocPtr(){return cfgDoc;}
	stlMutex* getMuxPtr(){return &docMux;}
	int saveDocument(const char *fn = NULL);
	STP getDocMem(int iFormat = 0);
	const char *getDocName(){ return (sFileName ? sFileName->sBuf : NULL); }

	STP getModuleParameter(const char* module,const char* parName);
	STP getModuleParameter(const char* module,const char* parName,const char* sDefault);
	STP getModuleParameterf(const char* module,const char* parName,...);
	STP getModuleParamAttrib(const char *sModule,const char *attrib,const char *parName,...);
	int getModuleParameterInt(const char* module,const char* parName);
	int getModuleParameterInt(const char* module,const char* parName,const int iDefault);
	double getModuleParameterDbl(const char* module,const char* parName);
	double getModuleParameterDbl(const char* module,const char* parName,const double dDefault);
	STP getConfigParameter(const char* parName,const char *sDefault = NULL);
	int getConfigParameterInt(const char* parName,const int iDefault = 0);
	STP getMachineConfigParameter(const char* parName,const char *sDefault = NULL);
	int getMachineConfigParameterInt(const char* parName,const int iDefault = 0);
	int setModuleParameter(const double dVal,const char* module,const char* parName,...);
	int setModuleParameter(const int iVal,const char* module,const char* parName,...);
	int setModuleParameter(const char *sVal,const char* module,const char* parName,...);
	int setModuleParameter(const ansStl::cST &sVal,const char* module,const char* parName,...);
	int setCANDeviceParameter(int canBus,int canID,const char* sParName,const char *value);
	int setCANDeviceParameter(int canBus,int canID,const char* sParName,int value);
	int setCANDeviceParameter(int canBus,int canID,const char* sParName,double value);
	int setCANDeviceParFactor(int canBus,int canID,const char* sParName,int iFactor);
	int setCANDeviceParAttrib(int canBus,int canID,const char* sParName,const char *attName,const char *fmt,...);
	STP getCANDeviceParameter(int canBus,int canID,const char* sParName,...);
	int getCANDeviceParFactor(int canBus,int canID,const char* sParName,...);
	bool isValid(){return (cfgDoc != NULL);}

protected:
	void loadXml(const char *fileName = NULL);
	void parseSTP(STP sXmlData);
private:
	STP cfgGetXpathAXML(const char *pStr,va_list args_lst);
	/* get xpath string(s) van configuratie
	*	/config/machine/grab/init/cam[@id='0']/frame_offset  (geeft camera offset camera ID=0)
	*	/config/machine/grab/init/camera/num_lines
	*/
	STP cfgGetXpathA(const char *pStr,va_list args_lst);
	int cfgDelXpathA(const char *pStr,va_list args_lst);
public:
	STP cfgGetXpathXML(const char *sXpath);
	STP cfgGetXpathXMLf(const char *fmt,...);
	STP cfgGetXpath(const char *sXpath);
	STP cfgGetXpathf(const char *fmt,...);
	int cfgGetXpathIntDf(int iDefault,const char *fmt,...);
	stlXmlList *cfgGetXpathListf(const char *fmt,...);

public:		// update of toevoegen van parameters aan xml
	/* Set xpath string van configuratie
	*	/config/machine/grab/init/cam[@id='0']/frame_offset  (set camera offset camera ID=0)
	*	/config/machine/grab/init/camera/num_lines
	* Return 0 Ok
	*       <0 Error
	*/
	int cfgSetXpath(const char *sValue,const char *sXpath);
	int cfgSetXpathf(const char *sValue,const char *sFmt,...);
public:			
	/* verwijderen delen van de xml
	** Return aantal nodes verwijderd
	*/
	int cfgDelXpathf(const char *sFmt,...);
	int cfgDelXpath(const char *sXpath);
};

#endif // __cplusplus

#endif // _STL_XML_H_
