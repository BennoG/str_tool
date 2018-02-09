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

#include <string.h>
#include <stdlib.h>

#include "stl_xml.h"
#include "stl_str.h"

/* Get specific node to textptr after an xpath (can also used to get node of attributes)
 * eg "item"
 * eg "item(attr)"
 * eg "level/item(attr)"
 */
xmlNodePtr stlXmlNodeSelect(xmlNodePtr cur,char *sPat)
{
	int i,j;
	STP sPath,Tm1;
	xmlAttrPtr atr;
	if (cur ==NULL) return NULL;
	if (sPat==NULL) return NULL;
	if (sPat[0]==0) return NULL;
	cur=cur->children;

	sPath=stlSetSt(sPat);
	stlConvert(sPath,'/',_D1);
	stlConvert(sPath,'(',_D2);
	stlConvert(sPath,')',_D2);

	i=j=1;
	Tm1=stlGetFld(sPath,i,1,0);
	while (cur){
		if (Tm1->iLen==0) break;
		if (cur->name){
			if (strcmp((const char*)cur->name,Tm1->sBuf)==0){	// Ok hit
				stlFree(Tm1);
				Tm1=stlGetFld(sPath,i,++j,0);	// property veld
				if (Tm1->iLen){
					atr=cur->properties;
					cur=NULL;
					while(atr){
						if (strcmp((const char*)atr->name,Tm1->sBuf)==0){	// Ok hit
							stlFree(Tm1);
							Tm1=stlGetFld(sPath,++i,1,0);j=1;
							cur=atr->children;
							break;
						}
						atr=atr->next;
					}
					// Geeft warning maar gaat op zig goed.
					//cur=cur->properties;
					continue;
				}
				stlFree(Tm1);
				Tm1=stlGetFld(sPath,++i,1,0);j=1;
				if ((Tm1->iLen==0)&&(cur->children==NULL)) break;	// Zodat lege string voor bestaand element kan worden terug gegeven.
				cur=cur->children;
				continue;
			}
		}
		cur=cur->next;
	}
	stlFree(Tm1  );
	stlFree(sPath);
	return cur;
}
/*

// stlXmlNodeStrSelect   -> stlXmlNodXPathStringf
// stlXmlNodeStrSelectNN -> stlXmlNodXPathStringfNN

// Zie stlXmlNodeSelect
// geeft echter de waarde terug i.p.v. de node (kan ook NULL terug geven als niet bestaat)
char *stlXmlNodeStrSelect(xmlNodePtr cur,char *sPath)
{
	cur=stlXmlNodeSelect(cur,sPath);
	if (cur==NULL) return NULL;
	if (cur->content==NULL) return "";
	return cur->content;
}

// kan meerdere antwoorden terug geven _FM delimited
STP stlXmlNodesSelect(xmlNodePtr cur,char *sPath,char *sNodeName)
{
	int iIdx=1;
	STP aAns;
	aAns=stlSetSt("");
	cur=stlXmlNodeSelect(cur,sPath);
	while (cur){
		if ((cur->type==1)&&(cur->name)){
			if (strcmp(cur->name,sNodeName)==0){
				if ((cur->children)&&(cur->children->type==3)&&(cur->children->content)){
					stlStoreStr(cur->children->content,aAns,iIdx++,0,0);
				}else{
					printf("#ERR5# stlXmlNodesSelect unexpected child on %s,%s",sPath,sNodeName);
				}
			}
		}
		cur=cur->next;
	}
	if (iIdx>1)	return aAns;
	stlFree(aAns);
	return NULL;
}

// Zie stlXmlNodeSelect
// geeft echter de waarde terug i.p.v. de node (geeft lege string terug als niet bestaat)
char *stlXmlNodeStrSelectNN(xmlNodePtr cur,char *sPath)
{
	cur=stlXmlNodeSelect(cur,sPath);
	if ((cur)&&(cur->content)) return cur->content;
	return "";
}
*/


static xmlXPathObjectPtr stlXmlXPathSelecta(xmlXPathContextPtr xptr,const char *fmt,va_list argptr)
{
	xmlXPathObjectPtr xoptr;
	STP sXpt;
	sXpt=stlSetSta(fmt,argptr);
	if (sXpt)
	{
		xoptr=xmlXPathEval((xmlChar*)(sXpt->sBuf),xptr);	// voer xpath selectie uit op document.
		stlFree(sXpt);
		if ((xoptr)&&(xoptr->nodesetval)&&(xoptr->nodesetval->nodeNr)) return xoptr;
		if (xoptr) xmlXPathFreeObject(xoptr);// Release allocated memory
	}
	return NULL;
}

static xmlNodePtr stlXmlXPathNodeA(xmlXPathContextPtr xptr,const char *fmt,va_list argptr)
{
	int i;
	xmlNodePtr nod=NULL;
	xmlXPathObjectPtr xoptr;
	if (xptr==NULL) return NULL;
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	if ((xoptr)&&(xoptr->nodesetval)&&(xoptr->nodesetval->nodeNr)){
		for (i=0;i<xoptr->nodesetval->nodeNr;i++){								// loop alle gevonden items van xpath door
			nod=xmlXPathNodeSetItem(xoptr->nodesetval,i);						// Geef geselecteerde node terug.
			if ((nod->name)&&(nod->type==XML_ATTRIBUTE_NODE)&&(nod->children)&&(nod->children->content))
				break;
			if ((nod->name)&&(nod->type==XML_ELEMENT_NODE  )&&(nod->children)&&(nod->children->content))
				break;
			nod=NULL;
		}
	}
	if (xoptr) xmlXPathFreeObject(xoptr);// Release allocated memory
	return nod;
}

/* Select xpath from document.
 *	
 */
static STP stlXmlXPathValueA(xmlXPathContextPtr xptr,const char *fmt,va_list argptr)
{
	xmlChar *pVal;
	int i;
	xmlNodePtr nod;
	xmlXPathObjectPtr xoptr;
	STP aRes=NULL;
	if (xptr==NULL) return NULL;
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	if ((xoptr)&&(xoptr->nodesetval)&&(xoptr->nodesetval->nodeNr)){
		for (i=0;i<xoptr->nodesetval->nodeNr;i++){								// loop alle gevonden items van xpath door
			pVal=NULL;
			nod=xmlXPathNodeSetItem(xoptr->nodesetval,i);						// Geef geselecteerde node terug.
			if ((nod->name)&&(nod->type==XML_ATTRIBUTE_NODE)&&(nod->children)&&(nod->children->content)){
				pVal=nod->children->content;
			}
			if ((nod->name)&&(nod->type==XML_ELEMENT_NODE  )&&(nod->children)&&(nod->children->content)){
				pVal=nod->children->content;
			}
			if (pVal)
			{
				if (aRes) stlAppendStf(aRes,"%c%s",_D1,pVal);
				else aRes=stlSetSt((char*)pVal);
			}
		}
	}
	if (xoptr) xmlXPathFreeObject(xoptr);// Release allocated memory
	return aRes;
}

STP stlXmlXPathValuef(xmlXPathContextPtr xptr,const char *fmt,...)
{
	va_list argptr;
	STP aRes;
	va_start(argptr, fmt);
	aRes=stlXmlXPathValueA(xptr,fmt,argptr);
	va_end(argptr);
	return aRes;
}

static xmlXPathContextPtr _stlNode2Context(xmlNodePtr nod)
{
	xmlXPathContextPtr xptr;
	if (nod==NULL) return NULL;
	xptr=xmlXPathNewContext(nod->doc);
	if (xptr==NULL) return NULL;
	xptr->node=nod;
	//xptr->here=nod;
	//xptr->origin=nod;
	return xptr;
}

const char* stlXmlNodXPathStringf(xmlNodePtr nod,const char *fmt,...)
{
	xmlNodePtr nRes;
	xmlXPathContextPtr xptr;
	va_list argptr;
	xptr=_stlNode2Context(nod);
	if (xptr==NULL) return NULL;
	va_start(argptr, fmt);
	nRes=stlXmlXPathNodeA(xptr,fmt,argptr);
	va_end(argptr);
	xmlXPathFreeContext(xptr);
	if ((nRes)&&(nRes->children->content)) return (const char *)(nRes->children->content);
	return NULL;
}
const char* stlXmlNodXPathNamef(xmlNodePtr nod,const char *fmt,...)
{
	xmlNodePtr nRes;
	xmlXPathContextPtr xptr;
	va_list argptr;
	xptr=_stlNode2Context(nod);
	if (xptr==NULL) return NULL;
	va_start(argptr, fmt);
	nRes=stlXmlXPathNodeA(xptr,fmt,argptr);
	va_end(argptr);
	xmlXPathFreeContext(xptr);
	if ((nRes)&&(nRes->name)) return (const char *)(nRes->name);
	return NULL;
}
const char* stlXmlXPathStringf(xmlXPathContextPtr xptr,const char *fmt,...)
{
	xmlNodePtr nRes;
	va_list argptr;
	va_start(argptr, fmt);
	nRes=stlXmlXPathNodeA(xptr,fmt,argptr);
	va_end(argptr);
	if ((nRes)&&(nRes->children->content)) return (const char *)(nRes->children->content);
	return NULL;
}
const char* stlXmlXPathStringfNN(xmlXPathContextPtr xptr,const char *fmt,...)
{
	xmlNodePtr nRes;
	va_list argptr;
	va_start(argptr, fmt);
	nRes=stlXmlXPathNodeA(xptr,fmt,argptr);
	va_end(argptr);
	if ((nRes)&&(nRes->children->content)) return (const char *)(nRes->children->content);
	return "";
}


const char* stlXmlNodXPathStringfNN(xmlNodePtr nod,const char *fmt,...)
{
	xmlNodePtr nRes;
	xmlXPathContextPtr xptr;
	va_list argptr;
	xptr=_stlNode2Context(nod);
	if (xptr==NULL) return "";
	va_start(argptr, fmt);
	nRes=stlXmlXPathNodeA(xptr,fmt,argptr);
	va_end(argptr);
	xmlXPathFreeContext(xptr);
	if ((nRes)&&(nRes->children->content))
		return (const char *)(nRes->children->content);
	return "";
}

STP stlXmlNodXPathValuef(xmlNodePtr nod,const char *fmt,...)
{
	xmlXPathContextPtr xptr;
	va_list argptr;
	STP aRes;
	xptr=_stlNode2Context(nod);
	if (xptr==NULL) return NULL;

	va_start(argptr, fmt);
	aRes=stlXmlXPathValueA(xptr,fmt,argptr);
	va_end(argptr);
	xmlXPathFreeContext(xptr);
	return aRes;
}

STP stlXmlDocXPathValuef(xmlDocPtr doc,const char *fmt,...)
{
	xmlXPathContextPtr xptr;
	va_list argptr;
	STP aRes;
	xptr=xmlXPathNewContext(doc);
	va_start(argptr, fmt);
	aRes=stlXmlXPathValueA(xptr,fmt,argptr);
	va_end(argptr);
	xmlXPathFreeContext(xptr);
	return aRes;
}





/* Selecteer xpath values uit de object.xml file
 * formatering volgens printf formaat
 */
xmlXPathObjectPtr stlXmlXPathSelectf(xmlXPathContextPtr xptr,const char *fmt,...)
{
	xmlXPathObjectPtr xoptr;
	va_list argptr;
	va_start(argptr, fmt);
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	va_end(argptr);
	return xoptr;
}
xmlXPathObjectPtr stlXmlNodXPathSelectf(xmlNodePtr xNod,const char *fmt,...)
{
	xmlXPathContextPtr xptr;
	xmlXPathObjectPtr xoptr;
	va_list argptr;

	xptr=_stlNode2Context(xNod);
	if (xptr==NULL) return NULL;

	va_start(argptr, fmt);
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	va_end(argptr);

	xmlXPathFreeContext(xptr);	// ik denk dat dit ok is (xoptr is hier niet van afhankelijk)
	return xoptr;
}



xmlNodePtr stlXmlDocXPathFirstNodef(xmlDocPtr doc,const char *fmt,...)
{
	xmlXPathContextPtr xptr;
	xmlXPathObjectPtr xoptr;
	xmlNodePtr nod=NULL;
	va_list argptr;
	xptr=xmlXPathNewContext(doc);
	va_start(argptr, fmt);
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	va_end(argptr);
	if (xoptr)
	{
		nod=xmlXPathNodeSetItem(xoptr->nodesetval,0);	// Geef geselecteerde node terug.
		xmlXPathFreeObject(xoptr);						// Release allocated memory
	}
	xmlXPathFreeContext(xptr);
	return nod;
}
/* Geeft node terug van xpath selectie
 *	Return NULL als niet gevonden
 */
xmlNodePtr stlXmlXPathFirstNodef(xmlXPathContextPtr xptr,const char *fmt,...)
{
	xmlNodePtr nod=NULL;
	xmlXPathObjectPtr xoptr;
	va_list argptr;
	va_start(argptr, fmt);
	xoptr=stlXmlXPathSelecta(xptr,fmt,argptr);
	va_end(argptr);
	if (xoptr)
	{
		nod=xmlXPathNodeSetItem(xoptr->nodesetval,0);	// Geef geselecteerde node terug.
		xmlXPathFreeObject(xoptr);						// Release allocated memory
	}
	return nod;
}



static void _fixStringSlash(STP sVal,char zoek,char rep)
{
	int i,iStr1 = 0;
	if (sVal == NULL) return;
	for (i = 0; i < sVal->iLen; i++)
	{
		char ch = sVal->sBuf[i];
		if (ch == '\'') iStr1 ^= 1;
		if ((ch == zoek) && (iStr1))
			sVal->sBuf[i] = rep;
	}
}

static STP stlGetDlmXP(STP pVA,int iD1,char cDlm)
{
	STP sRes = NULL;
	if (pVA == NULL) return sRes;
	_fixStringSlash(pVA,'/',1);
	sRes = stlGetDlm(pVA,iD1,cDlm);
	_fixStringSlash(pVA,1,'/');
	_fixStringSlash(sRes,1,'/');
	return sRes;
}
static void stlDelDlmXP(STP pVA,int iD1,char cDlm)
{
	if (pVA == NULL) return;
	_fixStringSlash(pVA,'/',1);
	stlDelDlm(pVA,iD1,cDlm);
	_fixStringSlash(pVA,1,'/');
}
static int stlCountXP(STP sV1,char ch)
{
	int iRes;
	_fixStringSlash(sV1,'/',1);
	iRes = stlCount(sV1,ch);
	_fixStringSlash(sV1,1,'/');
	return iRes;
}


static int stlXmlAddNodeTree(xmlXPathContextPtr xDoc,const char *sPat,xmlNodePtr *xAddedNode,xmlChar *xContent)
{
	xmlNodePtr  xNod;
	xmlXPathObjectPtr  xObjs;
	int iLvl,iErr=0,i;
	STP sPath,sNode,sName=NULL;
	sPath=stlSetSt(sPat);
	iLvl = stlCountXP(sPath,'/');
	//iLvl=stlCountChr(sPat,'/');
	if (iLvl==0){ stlFree(sPath); return -1; }
	
	sNode=stlGetDlmXP(sPath,iLvl+1,'/');
	stlDelDlmXP(sPath,iLvl+1,'/');
	if (strcmp(sNode->sBuf,"..")==0)
	{
		iErr=stlXmlAddNodeTree(xDoc,sPath->sBuf,xAddedNode,xContent);
		stlFree(sPath);
		stlFree(sNode);
		return iErr;
	}

	// sPath bevat voorloop path sNode node toe te voegen
	xObjs=stlXmlXPathSelectf(xDoc,sPath->sBuf);
	if (xObjs==NULL){
		if ((iErr=stlXmlAddNodeTree(xDoc,sPath->sBuf,NULL,xContent))) goto cleanup;
		xObjs=stlXmlXPathSelectf(xDoc,sPath->sBuf);
	}
	if ((xObjs)&&(xObjs->nodesetval)&&(xObjs->nodesetval->nodeNr)){
		xNod=xmlXPathNodeSetItem(xObjs->nodesetval,0);
		stlConvert(sNode,'[',_D1);stlConvert(sNode,']',_D1);  //          <1>  <2,1>  <2,2>  <3>  <4,1   <4,2>   <5>
		stlConvert(sNode,'=',_D2);                            // Zorg dat name[@attr='value']    [@attr='value'] word gesplitst

		// Controle voor [text()='lkjg'] voorwaarde (dan 1 level dieper gaan aanmaken)
		sName=stlGetFld(sNode,2,1,0);
		if (stricmp(sName->sBuf,"text()")==0)	// is een voorwaarde (ga 1 level dieper ook aanmaken)
		{
			stlFree(sName);
			sName=stlGetDlmXP(sPath,iLvl,'/');
			if (strcmp(sName->sBuf,"..")==0)
			{
				if ((iErr=stlXmlAddNodeTree(xDoc,sPath->sBuf,&xNod,xContent))) goto cleanup;
				if (xNod) xNod=xNod->parent;
			}else
			{
				xNod=xNod->parent;
				if (xNod) xNod=xmlNewTextChild(xNod,NULL,(xmlChar*)sName->sBuf,NULL);
			}
		}
		stlFree(sName);
		if (xNod==NULL) goto cleanup;			// er is iets fout gegaan 

		sName=stlGetFld(sNode,1,0,0);							// naam van child extracten
		if (strcmp(sName->sBuf,"..")==0) goto cleanup;			// deze nooit aanmaken
		if (sName->sBuf[0]=='@')
		{
			stlInsDel(sName,0,-1);
			xmlSetProp(xNod,(xmlChar*)sName->sBuf,xContent);
			goto cleanup;
		}
		xNod=xmlNewTextChild(xNod,NULL,(xmlChar*)sName->sBuf,NULL);		// element toevoegen

		for (i=2;;i+=2){
			// Attribute toevoegen indien nodig
			if (sName) stlFree(sName);
			sName=stlGetFld(sNode,i,1,0);
			if (sName->sBuf[0]=='@'){								// Ja moet attribute worden toegevoegd
				STP sValue;
				stlInsDel(sName,0,-1);
				sValue=stlGetFld(sNode,i,2,0); stlConvert(sValue,'\'',' '); stlStrip(sValue);
				xmlSetProp(xNod,(xmlChar*)sName->sBuf,(xmlChar*)sValue->sBuf);
				stlFree(sValue);
			}else if (stricmp(sName->sBuf,"text()")==0)
			{
				STP sValue;
				sValue=stlGetStr(sNode,i,2,0); stlConvert(sValue,'\'',' '); stlStrip(sValue);
				xmlNodeSetContent(xNod,(xmlChar*)sValue->sBuf);
				stlFree(sValue);
			}else{
				break;
			}
		}
	}

cleanup:
	if (xAddedNode) *xAddedNode=xNod;
	if (sPath) stlFree(sPath);
	if (sNode) stlFree(sNode);
	if (sName) stlFree(sName);
	if (xObjs) xmlXPathFreeObject(xObjs);			// Release allocated memory
	return iErr;
}

/* Add content to xml document (take notice on attributes)
 *  xDoc   document context
 *  sXpath xpath naar attribute
 *  xCont  value to store
 * Return 0 Ok
 *        1 Error
 */
int stlXmlAddUpdContent(xmlXPathContextPtr xDoc,const char *sXpath,xmlChar *xCont)
{
	int iRes=0;
	xmlNodePtr  xNod;
	xmlXPathObjectPtr  xObjs;
	if (xDoc){
		xObjs=stlXmlXPathSelectf(xDoc,sXpath);
		if (xObjs==NULL){
			stlXmlAddNodeTree(xDoc,sXpath,NULL,xCont);
			xObjs=stlXmlXPathSelectf(xDoc,sXpath);
		}
		if ((xObjs)&&(xObjs->nodesetval)&&(xObjs->nodesetval->nodeNr>=1)){
			xNod=xmlXPathNodeSetItem(xObjs->nodesetval,0);		// Voor elke beck unit
			xmlNodeSetContent(xNod,xCont);						// Zet waarde
		}else{
			printf("Error adding / updating node %s\n",sXpath);
			iRes=1;
		}
		if (xObjs) xmlXPathFreeObject(xObjs);					// Release allocated memory
	}else{
		iRes=1;
	}
	return iRes;
}

