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

#include "stl_str.h"


#define _DbFlgPrintErr_			0x0001	// Print all error messages.
#define _DbFlgSqlTimeout_		0x0002	// timeout if lock on connection takes too long.
#define _DbFlgPrintCmd_			0x0008	// Print alle dcommando's welke worden uitgevoerd
#define _DbFlgDoHeader_			0x0010	// Column header at first row
#define _DbFlgDescription_		0x0020	// Get description of columns

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* Timeout voor connection lock in ms
*/
extern int stlSqlLockTimeout;


/* Geef error welke bij de SQL server vandaan is gekomen
 *	iConIdx  SQL verbinding nummer
 * Return NULL (geen error geweest)
 *        data (melding)
 */
STP stlSqlConErrorGetMessage(int iConIdx);

/* Geef error welke bij de SQL server vandaan is gekomen
 * Return NULL (geen error geweest)
 *        data (melding)
 */
STP stlSqlErrorGetMessage(void);

/* Verbreek verbinding naar SQL server
 *	iConIdx  SQL verbinding nummer (default 0) (legal values 0-9)
 */
void stlSqlConDisconnect(int iConIdx);

/* Verbreek verbinding naar SQL server
 */
void stlSqlDisconnect(void);

/* Connect to a ODBC datasource.
**	sSrvr   ODBC dsn name
**  sUser   username for login
**  sPassw  password for login
**  sDbName database name to use
**  sDSNadd optional DSN parameters
**  iConIdx optional connection number (default 0) (legal values 0-9)
** Return  0 OK
**        -1 Out of memory (SQL env)
**        -2 Out of memory (SQL connect)
**        -3 Connect error
*/
int stlSqlConConnect(const char *sSrvr,const char *sUser,const char *sPassw,const char *sDbname,const char *sDSNadd,int iConIdx);
int stlSqlConnect(const char *sSrvr,const char *sUser,const char *sPassw,const char *sDbname);

/* Execute a SQL query on a specific connection
 *	sQuery  SQL query to execute (may contain ? params)
 *  iFlags  optional parameters
 *  iLstId  optional (To get value of auto increment column)
 *  sParam  optional Array of STP to determine ? params in query
 *  iParCnt optional Number STP's in sParam
 *  iConn   connection number to execute on (default 0) (legal values 0-9)
 */
STP stlSqlConQuery(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,int iConn);
STP stlSqlQuery(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt);


/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
STP stlSqlQueryf(const char *sCommand,...);

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
STP stlSqlParQueryf(STP *sParams,int iParCnt,const char *sCommand,...);

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlCommandf(const char *sCommand,...);
int stlSqlConCommandf(int iCon,const char *sCommand,...);

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlCommand(const char *sCommand);

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlParCommandf(STP *sParams,int iParCnt,const char *sCommand,...);

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlCommand(const char *sCommand);

/* Register new database engine to library
**   iConIdx      connection index number to use (default 0)
**   pPluginData  private data ptr for engine specific to this connection
**   pDoQuery     pointer to function for executing a query.
*/
int sqlAddEngine(int iConIdx,void *pPluginData,STP (*pDoQuery)(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,void *pPluginData,STP *pErrorMeld));



#ifdef __cplusplus
}
#endif // __cplusplus
