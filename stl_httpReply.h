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

#ifndef _STL_HTTP_REPLY_H_
#define _STL_HTTP_REPLY_H_

#ifdef  __cplusplus

#include "stl_str.h"
#include "stl_tcp.h"
#include <vector>

class stlHttpReply
{
private:
	void initVars(){httpCon=NULL;tcpError=0;bodyCode=0;body=NULL;tcpTimeout=10000;debugTcp=false;}
	typedef struct {STP sName;STP sValue;} varItem;
	int tcpTimeout;			// read timeout TCP connection
	stlTcpConn *httpCon;
	std::vector<STP> hdr;
	std::vector<varItem> bodyParam;
public:
	int tcpError;
	int bodyCode;
	STP body;
	bool debugTcp;
	stlHttpReply(){initVars();};
	stlHttpReply(stlTcpConn *stc,bool DebugTcp = false);
	~stlHttpReply();
private:
	int readHdr();
	void bodySplit();
public:
	STP hdrGet(const char *hzk);
	STP hdrGetVal(const char *hzk);
	// functions to get body data if body is type of url encoded answer
	const char *getBodyParamiSt(const char *bzk);
	STP getBodyParami(const char *bzk);
	int getBodyParamCount();
	STP getBodyParamIdxVal(int iIdx);
	STP getBodyParamIdxName(int iIdx);
};

#endif	// __cplusplus

#endif  // _STL_HTTP_REPLY_H_
