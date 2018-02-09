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
#ifndef _STL_HTTP_CLIENT_H_
#define _STL_HTTP_CLIENT_H_

#ifdef  __cplusplus

#include "stl_str.h"
#include "stl_tcp.h"
#include "stl_httpReply.h"
#include <vector>

class stlHttpClient
{
private:
	void initVars(){vMayor=vMinor=1; hHost=hHostBu=hGet=NULL; isBackupHost = false; ans = NULL; keepAlive=false; stc=NULL; poortNR=poortNrBu=80;tcpTimeout=10000; debugTcp = false;}
protected:
	std::vector<STP> hdr;
	int vMayor,vMinor;
	STP hHost,hGet;
	STP hHostBu;		// backup (fall-back) host naam als 1e geen antwoord geeft.
	bool isBackupHost;
	stlTcpConn* stc;
public:
	stlHttpReply *ans;
	int tcpTimeout;
	int poortNR,poortNrBu;
	bool keepAlive;
	bool debugTcp;
public:
	stlHttpClient(const char * url);
	stlHttpClient();
	~stlHttpClient();
	void setURL(const char *url);
	void setHost(const char * sHost);
	void setHostBackup(const char * sHost);
	int execute();

	int post(unsigned char *data,int datalen);
	// if consume is true the caller needs to delete the reply class (otherwise we are doing it)
	stlHttpReply *read(bool consume = false);

	void addTag(const char * sHdr,...);
	void updateTag(const char * sTag,...);
	void urlAppend(const char *fmt,...);

private:
	STP constructHeader(const char *type,...);
	int connect();		// make connection TCP
};

#endif	// __cplusplus

#endif  // _STL_HTTP_CLIENT_H_
