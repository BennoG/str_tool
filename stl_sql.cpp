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
#  include <windows.h>
#  pragma comment(lib, "odbc32.lib")
#endif

#include <string.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h> 

#include "stl_str.h"
#include "stl_sql.h"

#if defined(__linux__) || defined(__APPLE__)
#  include <signal.h>
#  define _lxBreak_() raise(SIGTRAP)
#endif
#ifdef _WIN32
#  define _lxBreak_() __debugbreak()
//#  define _lxBreak_() __asm int 3
#endif

/************************************************************************/
/*                                                                      */
/*                                                                      */
/* Interne functies en data structures                                  */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#define _MaxSqlCon_	10

#define _DbTypeMySql_			1
#define _DbTypePostgres_	2

#define _DbFlgRetryCmd_			0x0100	// SQL commando opnieuw uitvoeren.
#define _dbFlgReconnect_		0x0200	// Database verbinding opnieuw maken
#define _DbFlgSelectDb_			0x0400	// Database opnieuw activeren (mysql only ???)
#define _DbFlgPrintErrI_	   (0x0800 | _DbFlgPrintErr_)	// zodat deze flag bij de connectie bewaard blijft

static stlMutex sqlMux=stlMutexInitializer;

static struct sqlConTableStruct{
	stlMutex conMux;
	STP		sDbInfo;
	SQLHENV	 hEnv;
	SQLHDBC	 hDbc;
	int		iDbType;			// Soort database dat gebruitk word.
	int		iDbFlags;
	// Data welke per query word gezet
	STP   sQuery;				// Query welke uit gevoerd word
	STP   *sParm;				// Parameter array
	int   iParCnt;			// Aantal parameters
	// Error afhandeling en opslag
	stlMutex errMux;
	STP		sErrMld;
	// voor esternal SQL engines (not ODBC like sqlite etc)
	void *_pPluginData;
	STP (*_sqlDoQuery)(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,void *pPluginData,STP *pErrorMeld);

} _sqlConTable[_MaxSqlCon_];

int stlSqlLockTimeout=0;

static STP _sqlDoQuery(struct sqlConTableStruct *pCon,int *lstId);
static int _sqlGetLastID(struct sqlConTableStruct *pCon);

/* Initialiseer geheugen voor SQL verbindingen
 */
static void _sqlInit(void)
{
	int i;
	static int iInit=0;
	if (iInit) return;
	stlMutexLock(&sqlMux);
	if (iInit==0){
		iInit=1;
		memset(_sqlConTable,0,sizeof(_sqlConTable));
		for (i=0;i<_MaxSqlCon_;i++){
			stlMutexInit(&_sqlConTable[i].conMux);
			stlMutexInit(&_sqlConTable[i].errMux);
		}
	}
	stlMutexUnlock(&sqlMux);
}

/* Genereer Error melding welke bij server vandaan komt (of uit odbc)
 *	
 */
static void _sqlSetError(struct sqlConTableStruct *pCon,SQLSMALLINT iHdlType,SQLHANDLE iHandle)
{
	SQLSMALLINT iMsgLen,iRec=1;
	SQLINTEGER iNativeErr;
	SQLCHAR sState[8],sErr[SQL_MAX_MESSAGE_LENGTH];
	stlMutexLock(&pCon->errMux);
	if (pCon->sErrMld) stlFree(pCon->sErrMld);
	pCon->sErrMld=NULL;
	if (pCon->iDbFlags & _DbFlgPrintErrI_) printf("SQL Error query '%s'\n",pCon->sQuery?pCon->sQuery->sBuf:"null");
	
	while (SQLGetDiagRec(iHdlType, iHandle, iRec, sState, &iNativeErr,sErr, sizeof(sErr), &iMsgLen) != SQL_NO_DATA){
		if (pCon->iDbType == _DbTypeMySql_){
			switch (iNativeErr){
			case 1046: pCon->iDbFlags |= _DbFlgSelectDb_;  break;	// no database selected.
			case 2006: pCon->iDbFlags |= _dbFlgReconnect_; break;	// MySQL server has gone away
			case 2003: pCon->iDbFlags |= _dbFlgReconnect_; break;	// Can't connect to MySQL server
			case 2013: pCon->iDbFlags |= _DbFlgRetryCmd_;  break;	// Lost connection to MySQL server during query
			}
		}
		if (pCon->sErrMld) stlAppendStf(pCon->sErrMld,"%s %d %s\n",sState,(int)iNativeErr,sErr);
		else pCon->sErrMld=stlSetStf("%s %d %s\n",sState,(int)iNativeErr,sErr);
		if (pCon->iDbFlags & _DbFlgPrintErrI_) printf("%s %d %s\n",sState,(int)iNativeErr,sErr);
		if (iRec++>10) break;
	}
	stlMutexUnlock(&pCon->errMux);
}
static void _sqlErrStrf(struct sqlConTableStruct *pCon,const char *sFmt,...)
{
	va_list ap;
	va_start(ap,sFmt);
	if (pCon->sErrMld) stlFree(pCon->sErrMld);
	pCon->sErrMld=stlSetSta(sFmt,ap);
	va_end(ap);
	if (pCon->iDbFlags & _DbFlgPrintErrI_) printf("%s\n",pCon->sErrMld->sBuf);
}


/* Haal verbinding data op
 *  iConIdx  index nummer van SQL verbinding
 */
static struct sqlConTableStruct * _sqlGetCon(int iConIdx)
{
	_sqlInit();
	if ((iConIdx<0)||(iConIdx>=_MaxSqlCon_)) return &_sqlConTable[0];
	return &_sqlConTable[iConIdx];
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/* Publieke functies                                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/* Geef error welke bij de sql server vandaan is gekomen
 *	iConIdx  sql verbinding nummer
 * Return NULL (geen error geweest)
 *        data (melding)
 */
STP stlSqlConErrorGetMessage(int iConIdx)
{
	STP sErr;
	struct sqlConTableStruct *pCon;
	pCon=_sqlGetCon(iConIdx);
	stlMutexLock(&pCon->errMux);
	sErr=pCon->sErrMld;
	pCon->sErrMld=NULL;
	stlMutexUnlock(&pCon->errMux);
	return sErr;
}
/* Geef error welke bij de sql server vandaan is gekomen
 * Return NULL (geen error geweest)
 *        data (melding)
 */
STP stlSqlErrorGetMessage(void)
{
	return stlSqlConErrorGetMessage(0);
}

/* Verbreek verbinding naar sql server
 *	iConIdx  sql verbinding nummer
 */
void stlSqlConDisconnect(int iConIdx)
{
	struct sqlConTableStruct *pCon;
	pCon=_sqlGetCon(iConIdx);
	stlMutexLock  (&pCon->conMux);
	if (pCon->hDbc){
		SQLDisconnect (pCon->hDbc);
		SQLFreeConnect(pCon->hDbc);
	}
	if (pCon->hEnv){
		SQLFreeEnv    (pCon->hEnv);
	}
	pCon->hEnv = 0;
	pCon->hDbc = 0; 
	stlMutexUnlock(&pCon->conMux);
}

/* Verbreek verbinding naar sql server
 */
void stlSqlDisconnect(void)
{
	stlSqlConDisconnect(0);
}




/* Internal connect to a database
**	
** Return  0 OK
**        -1 Out of memory (SQL env)
**        -2 Out of memory (SQL connect)
**        -3 Connect error
*/
static int _sqlConnect(struct sqlConTableStruct *pCon)
{
	SQLHENV	hLocEnv = 0;
	SQLHDBC	hLocDbc = 0;
	SQLSMALLINT iResLen;
	SQLRETURN iRet;
	STP sDsn,sRes,sOldQuery;
	int iOldPartCnt;
	//	STP aCon,Tm1,aRes,aCmd;

	// Eventuele bestaande verbinding opruimen
	if (pCon->hDbc){
		SQLDisconnect (pCon->hDbc);
		SQLFreeConnect(pCon->hDbc);
	}
	if (pCon->hEnv){
		SQLFreeEnv    (pCon->hEnv);
	}
	pCon->hDbc=NULL;
	pCon->hEnv=NULL;

	// opbouwen van verbinding
	iRet=SQLAllocEnv(&hLocEnv);
	if (iRet != SQL_SUCCESS){
		_sqlErrStrf(pCon,"Error: SQLAllocEnv returned %d\n",iRet);
		return -1;
	}
	
	iRet=SQLAllocConnect( hLocEnv, &hLocDbc );
	if (iRet != SQL_SUCCESS){
		_sqlSetError(pCon,SQL_HANDLE_ENV,hLocEnv);
		_sqlErrStrf (pCon,"Error: SQLAllocConnect returned %d\n",iRet);
		SQLFreeEnv( hLocEnv );
		return -2;
	}
	
	// Eigenlijke connect naar database
	sDsn=stlGetStr(pCon->sDbInfo,1,0,0);
	sRes=stlInitLen(10240,0);
	iRet=SQLDriverConnect(hLocDbc,NULL,(unsigned char*)sDsn->sBuf,(SQLSMALLINT)sDsn->iLen,(unsigned char *)sRes->sBuf,(SQLSMALLINT)sRes->iLen,&iResLen,SQL_DRIVER_NOPROMPT);
	if (iRet) printf("DSNERR=%s\n",sDsn->sBuf);
#ifdef _DEBUG	
	else      printf("DSNOK=%s\n" ,sDsn->sBuf);
	printf("connect ret=%d env=%X dbc=%X\n",iRet,(int)(uint64_t)hLocEnv,(int)(uint64_t)hLocDbc);
#endif
	stlFree(sRes);
	stlFree(sDsn);

	// Controleer of verbinding juist tot stand is gekomen
	if (iRet != SQL_SUCCESS){
		_sqlSetError(pCon,SQL_HANDLE_DBC,hLocDbc);
		SQLFreeConnect(hLocDbc);
		SQLFreeEnv(hLocEnv);
#ifdef _DEBUG
		_lxBreak_();
#endif
		return -3;
	}
	pCon->hEnv=hLocEnv;
	pCon->hDbc=hLocDbc;
	pCon->iDbType=0;

	sOldQuery   = pCon->sQuery;	// bewaar commando waar we mee bezig waren.
	iOldPartCnt = pCon->iParCnt;

	pCon->iParCnt=0;
	pCon->sQuery =stlSetSt("select version()");
	sRes=_sqlDoQuery(pCon,NULL);
	if (sRes){
		strupr(sRes->sBuf);
		if (strstr(sRes->sBuf,"POSTGRES")) pCon->iDbType=_DbTypePostgres_;
		stlFree(sRes);
	}
	if (pCon->iDbType==0) pCon->iDbType=_DbTypeMySql_;
	stlFree(pCon->sQuery);
	pCon->sQuery=NULL;

	if (pCon->iDbType==_DbTypeMySql_){
		sDsn=stlGetStr(pCon->sDbInfo,2,0,0);
		pCon->iParCnt=0;
		pCon->sQuery =stlSetStf("use %s",sDsn->sBuf);
		sRes=_sqlDoQuery(pCon,NULL);
		stlFree(sRes);
		stlFree(sDsn);
		stlFree(pCon->sQuery);
		pCon->sQuery=NULL;
	}
	pCon->sQuery = sOldQuery;
	pCon->iParCnt= iOldPartCnt;
	return 0;
}
/* Connect to a ODBC datasource.
**	sSrvr   ODBC dsn name
**  sUser   username for login
**  sPassw  password for login
**  sDbName database name to use
** Return  0 OK
**        -1 Out of memory (SQL env)
**        -2 Out of memory (SQL connect)
**        -3 Connect error
*/
int stlSqlConConnect(const char *sSrvr,const char *sUser,const char *sPassw,const char *sDbname,const char *sDSNadd,int iConIdx)
{
	int iRes;
	STP sDsn;
	struct sqlConTableStruct *pCon;
	pCon=_sqlGetCon(iConIdx);

	if (sDbname==NULL) sDbname="";
	sDsn=stlSetSt("");

	// "PROVIDER=SQLOLEDB;DATASOURCE=mssql.domain.com;UID=username;PWD=password;DATABASE=mydatabase "
	if ((sSrvr  )&&(sSrvr  [0])) stlAppendStf(sDsn,"%sDSN=%s"     ,sDsn->iLen?";":"",sSrvr  );
	if ((sUser  )&&(sUser  [0])) stlAppendStf(sDsn,"%sUID=%s"     ,sDsn->iLen?";":"",sUser  );
	if ((sPassw )&&(sPassw [0])) stlAppendStf(sDsn,"%sPWD=%s"     ,sDsn->iLen?";":"",sPassw );
	if ((sDbname)&&(sDbname[0])) stlAppendStf(sDsn,"%sDATABASE=%s",sDsn->iLen?";":"",sDbname);
	if ((sDSNadd)&&(sDSNadd[0])) stlAppendStf(sDsn,"%s%s"         ,sDsn->iLen?";":"",sDSNadd);

	stlStoreStr(sDbname,sDsn,2,0,0);
	stlMutexLock(&sqlMux);			// MySQL/ODBC crashed when multiple threads do simultane connects
	stlMutexLock(&pCon->conMux);
	if (pCon->sDbInfo) stlFree(pCon->sDbInfo);
	pCon->iDbFlags|= _DbFlgPrintErrI_;
	pCon->sDbInfo  = sDsn;
	iRes=_sqlConnect(pCon);
	stlMutexUnlock(&pCon->conMux);
	stlMutexUnlock(&sqlMux);
	return iRes;
}
int stlSqlConnect(const char *sSrvr,const char *sUser,const char *sPassw,const char *sDbname)
{
	return stlSqlConConnect(sSrvr,sUser,sPassw,sDbname,NULL,0);
}


/************************************************************************/
/* Plugin stuff                                                         */
/************************************************************************/

/* Register new database engine to library
**   iConIdx      connection index number to use (default 0)
**   pPluginData  private data ptr for engine specific to this connection
**   pDoQuery     pointer to function for executing a query.
*/
int sqlAddEngine(int iConIdx,void *pPluginData,STP (*pDoQuery)(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,void *pPluginData,STP *pErrorMeld))
{
	struct sqlConTableStruct *pCon;
	if ((iConIdx<0)||(iConIdx>=_MaxSqlCon_)) return -1;
	pCon=_sqlGetCon(iConIdx);
	pCon->_pPluginData = pPluginData;
	pCon->_sqlDoQuery  = pDoQuery;
	return 0;
}




/************************************************************************/
/*                                                                      */
/************************************************************************/


static STP _stlSqlGetHdr( SQLHSTMT hStmt)
{
	STP aHdr;
	SQLSMALLINT nCol,nColumns=0;
	SQLCHAR  sBuf[1024];
	
	if (SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS)  return NULL;

	aHdr=stlSetSt("");
	for( nCol = 1; nCol <= nColumns; nCol++ ){
		SQLColAttribute( hStmt, nCol, SQL_DESC_LABEL, sBuf,(SQLSMALLINT) sizeof(sBuf)-1 , NULL, NULL );
		if (nCol>1) stlAppendCh(aHdr,_D2);
		stlAppendSt(aHdr,(char*)sBuf);
	}
	return aHdr;
}

static STP _stlSqlGetDescript( SQLHSTMT hStmt)
{
	STP aHdr;
	SQLSMALLINT nRow,nCol,nColumns=0;
	SQLCHAR  sBuf[1024];
	
	if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )  return NULL;

	aHdr=stlSetSt("");
	for( nCol = 1; nCol <= nColumns; nCol++ ){
		// TODO: hier nog omschrijving van kolom naam invoegen.
		for (nRow=1;nRow<=35;nRow++){	// Alle types
			SQLINTEGER iRes;
			SQLLEN iVal=123456;
			sBuf[0]=0;
			iRes=SQLColAttribute( hStmt, nCol, nRow, sBuf,(SQLSMALLINT) sizeof(sBuf)-1,NULL,&iVal);
			if (iRes==SQL_SUCCESS){
				if (sBuf[0]) stlStoreStr((char*)sBuf,aHdr,nCol,nRow,0);
				else         stlStoreInt(iVal,aHdr,nCol,nRow,0);
			}else{
				stlStoreStr("*ERROR*",aHdr,nCol,nRow,0);
			}
		}
	}
	return aHdr;
}

static STP _stlSqlGetData( SQLHSTMT hStmt,struct sqlConTableStruct *pCon)
{
	STP aRes,aRow;//,aVld;
	SQLSMALLINT  nCol,nType,nReturn=0,nColumns=0;
	SQLLEN       nTypA[3], nIndicator = 0;
	bool realUtf8 = false;
	
	if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )  return NULL;

	//aVld=stlInitLen(1024,0);		// BG 2013-09-25
	aRes=stlSetSt("");
	/* ROWS */
	while(( nReturn = SQLFetch( hStmt )) == SQL_SUCCESS ){
		aRow=stlSetSt("");
		if (aRes->iLen) stlAppendCh(aRow,_D1);
		/* COLS */
		for( nCol = 1; nCol <= nColumns; nCol++ ){
			char buf[5];
			if (nCol>1) stlAppendCh(aRow,_D2);
			nType=1;
			// SQL_COLUMN_TYPE       1 = blob/txt 4=int
			// SQL_DESC_TYPE         -4=blob 4=int 12=varchar
			// SQL_DESC_CONCISE_TYPE 1 = blob/txt 4=int
			// Bugfux (als op utf8 gecompileerd en gerunt op utf16 word 1 integer extra op stack overschreven)
			//        ( hierdoor werd ncolumns overschreven)
			nTypA[0]=nType;
			SQLColAttribute( hStmt, nCol, SQL_DESC_TYPE, NULL,0, NULL, &nTypA[0]);
			nType=(SQLSMALLINT)(nTypA[0]);
			switch (nType)
			{
			case SQL_WCHAR:
			case SQL_WVARCHAR:
			case SQL_WLONGVARCHAR:
				nType = SQL_C_WCHAR;
				break;
			case SQL_LONGVARBINARY:
			case SQL_VARBINARY:
				nType=SQL_C_BINARY;
				break;
			default:
				nType=SQL_C_CHAR;
				break;
			}
			// 2017-09-29 BG voor UTF8 ondersteuning
			//if ((nType==SQL_LONGVARBINARY)||(nType==SQL_VARBINARY)) nType=SQL_C_BINARY; else nType=SQL_C_CHAR;

			nReturn = SQLGetData( hStmt, nCol, nType, buf,(SQLINTEGER) 0, &nIndicator );
			if ( nReturn == SQL_ERROR ){
				_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
				_sqlErrStrf (pCon,"Error: SQLGetData returned %d\n",nReturn);
				break;
			}
			if ((nReturn == SQL_SUCCESS || nReturn==SQL_SUCCESS_WITH_INFO) && nIndicator != SQL_NULL_DATA ){
				if (nIndicator == SQL_NO_TOTAL)
				{
					STP aVld=stlInitLen(1024,0);
					while (aVld->iLen>=1022){
						nReturn = SQLGetData( hStmt, nCol, nType, aVld->sBuf,(SQLINTEGER) 1024, &nIndicator );
						if (nReturn == SQL_NO_DATA) break;
						if (nReturn == SQL_ERROR  ){
							_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
							_sqlErrStrf (pCon,"Error: SQLGetData returned %d\n",nReturn);
							break;
						}
						aVld->iLen = (nIndicator > 1024) || (nIndicator == SQL_NO_TOTAL) ? 1022 : nIndicator;
						if ((aVld->iLen==1022)&&(aVld->sBuf[1022])) aVld->iLen=1023;
						stlAppendStp(aRow,aVld);
					}
					stlFree(aVld);
					printf("_stlSqlGetData return SQL_NO_DATA");
#ifdef _DEBUG
					_lxBreak_();
#endif
				}else if (nType == SQL_WCHAR)
				{
					if (nIndicator & 1) _lxBreak_();

					ansStl::cST aVld(nIndicator / 2,(wchar_t*)NULL);
					nReturn = SQLGetData( hStmt, nCol, nType,(SQLPOINTER) aVld.bufW(),(SQLINTEGER) (aVld.length() + 1) * 2 , &nIndicator );
					if ((nReturn == SQL_ERROR  ) || (nReturn != SQL_SUCCESS)){
						_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
						_sqlErrStrf (pCon,"Error: SQLGetData returnd %d\n",nReturn);
					}else{
						STP v = aVld.getStp();
						stlAppendStp(aRow,v);
						stlFree(v);
						if (aVld.isUTF8) realUtf8 = true;
					}
				}else
				{
					STP aVld = stlInitLen(nIndicator,0);	// binaire data exacte lengte anders + 1 voor trailing 0
					nReturn = SQLGetData( hStmt, nCol, nType, aVld->sBuf,(SQLINTEGER) aVld->iLen + (nType == SQL_C_CHAR ? 1 : 0) , &nIndicator );
					if ((nReturn == SQL_ERROR  ) || (nReturn != SQL_SUCCESS)){
						_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
						_sqlErrStrf (pCon,"Error: SQLGetData returnd %d\n",nReturn);
					}else
						stlAppendStp(aRow,aVld);
					stlFree(aVld);
				}
			}
		}
		stlAppendStp(aRes,aRow);
		stlFree(aRow);
	}
	if (realUtf8)
	{
		stlInsDel(aRes,0,3);
		aRes->sBuf[0] = (char)0xEF;
		aRes->sBuf[1] = (char)0xBB;
		aRes->sBuf[2] = (char)0xBF;
	}
	return aRes;
}



#define _MaxItems_ 20

static STP _sqlDoQuery(struct sqlConTableStruct *pCon,int *lstId)
{
	int i,iTime;
	STP sRes,sTmp;
	SQLSMALLINT iRes,cols;
	SQLHSTMT	  hStmt;
	SQLPOINTER  pToken;
	SQLLEN  iLen[_MaxItems_];

#ifdef _DEBUG
	pCon->iDbFlags |= _DbFlgPrintErrI_;
#endif

	// Reconnect ???
	if ((pCon->hDbc==0)&&(pCon->sDbInfo)) _sqlConnect(pCon);
	if (pCon->hDbc==0) return NULL;

	// Bereid sql commando voor
	iRes=SQLAllocStmt(pCon->hDbc,&hStmt);
	if (iRes != SQL_SUCCESS){
		_sqlSetError(pCon,SQL_HANDLE_DBC,pCon->hDbc);
		_sqlErrStrf (pCon,"Error: SQLAllocStmt returnd %d\n",iRes);
		return NULL;
	}

	iRes=SQLPrepare(hStmt,(unsigned char *)(pCon->sQuery->sBuf),pCon->sQuery->iLen);
	if (iRes != SQL_SUCCESS){
		_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
		_sqlErrStrf(pCon,"Error: SQLAllocStmt returnd %d\n",iRes);
		SQLFreeStmt(hStmt,SQL_DROP);
		return NULL;
	}

	if (pCon->iDbFlags & _DbFlgPrintCmd_) printf("CMD=%s\n",pCon->sQuery->sBuf);
	
	// voorbereiden van de parameters
	if (pCon->iParCnt>_MaxItems_) pCon->iParCnt=_MaxItems_;
	for (i=0;i<pCon->iParCnt;i++){
		if (pCon->sParm[i]){
			iLen[i]=pCon->sParm[i]->iLen;
			if (memchr(pCon->sParm[i]->sBuf,0,pCon->sParm[i]->iLen)){	// Het is binaire data
				iRes=SQLBindParameter(hStmt,(SQLSMALLINT)(i+1),SQL_PARAM_INPUT,SQL_C_BINARY,SQL_VARBINARY  ,pCon->sParm[i]->iLen,0,pCon->sParm[i]->sBuf,0,&iLen[i]);
			}else{										// Het is ASCII data
				iRes=SQLBindParameter(hStmt,(SQLSMALLINT)(i+1),SQL_PARAM_INPUT,SQL_C_CHAR  ,SQL_LONGVARCHAR,pCon->sParm[i]->iLen,0,pCon->sParm[i]->sBuf,0,&iLen[i]);
			}
			iLen[i]=pCon->sParm[i]->iLen;
		}else{
			iLen[i]=SQL_NULL_DATA;
			//iRes=SQLBindParameter(hStmt,(SQLSMALLINT)(i+1),SQL_PARAM_INPUT,SQL_C_CHAR,SQL_LONGVARCHAR,0,0,(SQLPOINTER)(i+1),0,&iLen[i]);
			iRes=SQLBindParameter(hStmt,(SQLSMALLINT)(i+1),SQL_PARAM_INPUT,SQL_C_NUMERIC,SQL_NUMERIC,0,0,NULL,0,&iLen[i]);	// volgens ODBC documentatie
		}
		if (iRes != SQL_SUCCESS){
			_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
			_sqlErrStrf (pCon,"Error: SQLBindParameter returnd %d\n",iRes);
		}
	}

	for (i=0;i<pCon->iParCnt;i++){
		if (pCon->iDbFlags & _DbFlgPrintCmd_) printf("PAR%d=%s\n",i,pCon->sParm[i]?pCon->sParm[i]->sBuf:"NULL");
	}


	// Eigenlijke uitvoeren van commando
	iTime=stlMsTimer(0);
	iRes=SQLExecute( hStmt );
	if (iRes == SQL_NEED_DATA ){
		while (iRes == SQL_NEED_DATA ){
			iRes = SQLParamData(hStmt,&pToken);
			if (iRes == SQL_NEED_DATA ){
				i=((int)((int64_t)pToken))-1;
				if ((i<0)||(i>=pCon->iParCnt)){
					SQLPutData(hStmt,NULL,0);
				}else{
					if (pCon->sParm[i]) SQLPutData(hStmt,pCon->sParm[i]->sBuf,pCon->sParm[i]->iLen);
					else         SQLPutData(hStmt,NULL,0);
				}
			}
		}
	}
	iTime=stlMsTimer(iTime);

	// Als query langer dan 500ms duurd melding genereren
	if (iTime>500){
		STP sQr=stlCopy(pCon->sQuery);
		stlConvert(sQr,'\n',' ');
		stlConvert(sQr,'\t',' ');
		stlConvert(sQr,'\r',' ');
		stlSingle (sQr,' ');
		if (sQr==NULL) sQr=stlSetSt("NULL");
		if (sQr->iLen > 250) sQr->iLen=250; sQr->sBuf[sQr->iLen]=0;
		printf("Warning SQL Query %d ms %s\n",iTime,sQr->sBuf);
		stlFree(sQr);
	}
	
	if (iRes==SQL_SUCCESS_WITH_INFO) iRes=SQL_SUCCESS;
	if (iRes != SQL_SUCCESS )
	{
		_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
		_sqlErrStrf (pCon,"Error: SQL execution error %d\n",iRes);
		SQLFreeStmt (hStmt,SQL_DROP);
		return NULL;
	}

//	if (lstId) *lstId=_sqlGetLastID(pCon);

	// Controleer of we een result set hebben
	iRes=SQLNumResultCols(hStmt,&cols);
	if (iRes != SQL_SUCCESS)
	{
		_sqlSetError(pCon,SQL_HANDLE_STMT,hStmt);
		_sqlErrStrf (pCon,"Error: SQLNumResultCols returnd %d\n",iRes);
		SQLFreeStmt (hStmt,SQL_DROP );
		return NULL;
	}

	sRes=NULL;
	if (cols > 0){
		// TODO: deze omdraaien met header
		if (pCon->iDbFlags & _DbFlgDescription_)
		{
			sTmp=_stlSqlGetDescript(hStmt);
			if ((sRes)&&(sTmp))
			{
				stlAppendCh(sRes,_D1);
				stlAppendStp(sRes,sTmp);
				stlFree(sTmp);
			}else
			{
				if (sTmp) sRes=sTmp;
			}
		}
		if (pCon->iDbFlags & _DbFlgDoHeader_)
		{
			sTmp=_stlSqlGetHdr(hStmt);
			if ((sRes)&&(sTmp))
			{
				stlAppendCh(sRes,_D1);
				stlAppendStp(sRes,sTmp);
				stlFree(sTmp);
			}else
			{
				if (sTmp) sRes=sTmp;
			}
		}
		sTmp=_stlSqlGetData(hStmt,pCon);
		if ((sRes)&&(sTmp))
		{
			stlAppendCh(sRes,_D1);
			stlAppendStp(sRes,sTmp);
			stlFree(sTmp);
		}else
		{
			if (sTmp) sRes=sTmp;
		}
	}else{
		SQLLEN  nRowsAffected	= -1;
		SQLRowCount( hStmt,&nRowsAffected );
		sRes=stlSetStf("%d rows affected\n", nRowsAffected);
	}

	// Cleanup
	SQLFreeStmt( hStmt, SQL_DROP );
	if ((sRes)&&(sRes->iLen==0))
	{
		stlFree(sRes);
		sRes=NULL;
	}
	
	if (lstId) *lstId=_sqlGetLastID(pCon);

//	if (pCon->iDbFlags & _DbFlgPrintCmd_) DumpAbp("RES",sRes);
	return sRes;
}

/* Haal ID op van laatste insert commando Voor autoincrement colommen.
 *	pCon  connection struct
 * Return <0 error
 *        >0 ID van laatste insert
 */
static int _sqlGetLastID(struct sqlConTableStruct *pCon)
{
	int res=-1,iOldPartCnt;
	STP Tm1,Tm2,sOldQuery;
	sOldQuery  = pCon->sQuery;
	iOldPartCnt= pCon->iParCnt;
	switch (pCon->iDbType){
	case _DbTypeMySql_:
		pCon->iParCnt=0;
		pCon->sQuery =stlSetSt("SELECT LAST_INSERT_ID()");
		Tm1=_sqlDoQuery(pCon,NULL);
		res=stlGetInt(Tm1,1,1,1);
		stlFree(Tm1);
		stlFree(pCon->sQuery);
		break;
	case _DbTypePostgres_:
		Tm1=stlCopy(pCon->sQuery);
		stlConvert(Tm1,9,32);stlConvert(Tm1,10,32);stlConvert(Tm1,13,32);stlSingle(Tm1,' ');stlStrip(Tm1);	// Alles omzetten naar spaties enkel
		Tm2=stlGetDlm(Tm1,3,' ');			// Table naam is 3 woord in insert into xxxxx
		pCon->iParCnt=0;
		pCon->sQuery =stlSetStf("SELECT relname FROM pg_class WHERE relname LIKE '%s_%%_seq' LIMIT 1",Tm2->sBuf);
		stlFree(Tm1);
		stlFree(Tm2);
		Tm2=_sqlDoQuery(pCon,NULL);
		stlFree(pCon->sQuery);
		if (Tm2){
			pCon->iParCnt=0;
			pCon->sQuery =stlSetStf("SELECT currval('%s')",Tm2->sBuf); 
			Tm1=_sqlDoQuery(pCon,NULL);
			res=stlGetInt(Tm1,1,1,1);
			stlFree(Tm1);
			stlFree(Tm2);
			stlFree(pCon->sQuery);
		}
		break;
	}
	pCon->sQuery  = sOldQuery;
	pCon->iParCnt = iOldPartCnt;
	return res;
}

/* Execute a SQL query on a specific connection
 *	sQuery  Sql query to execute (may contain ? params)
 *  iFlags  optional parameters
 *  iLstId  optional (To get value of autoincrement column)
 *  sParam  optional Array of STP to determine ? params in query
 *  iParCnt optional Number STP's in sParam
 *  iConn   connection number to execute on (default 0) (legal values 0-9)
 */
STP stlSqlConQuery(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,int iConn)
{
	STP aRes = NULL, sQshort = NULL;
	struct sqlConTableStruct * pCon;

	int iTimeT;
	int iTime=stlMsTimer(0);

	// verkorte query maken voor rapportage doeleinden
	sQshort=stlCopy(sQuery);
	stlConvert(sQshort,'\n',' ');
	stlConvert(sQshort,'\t',' ');
	stlConvert(sQshort,'\r',' ');
	stlSingle (sQshort,' ');
	if (sQshort==NULL) sQshort=stlSetSt("NULL");
	if (sQshort->iLen > 250) sQshort->iLen=250; sQshort->sBuf[sQshort->iLen]=0;

	pCon=_sqlGetCon(iConn);
	if ((pCon->iDbFlags & _DbFlgSqlTimeout_) || (stlSqlLockTimeout>0))
	{
		int iRes=stlMutexLockTo(&pCon->conMux,stlSqlLockTimeout>0?stlSqlLockTimeout:1000);
		if (iRes)
		{
			printf("stlSqlConQuery lock timeout sql '%s'",sQshort->sBuf);
			stlFree(sQshort);
			return NULL;
		}
	}else
		stlMutexLock(&pCon->conMux);

	iTimeT = stlMsTimer(iTime);

	stlMutexLock(&pCon->errMux);
	stlFree(pCon->sErrMld);
	pCon->sErrMld=NULL;
	pCon->sQuery =sQuery;
	pCon->sParm  =sParams;
	pCon->iParCnt=iParCnt;
	stlMutexUnlock(&pCon->errMux);
	
	iTime = stlMsTimer(iTime);
	if (iTime>500) printf("Warning SQL lock %d,%d ms '%s'\n",iTimeT,iTime,sQshort->sBuf);

	if (pCon->_sqlDoQuery)
	{
		iTime=stlMsTimer(0);
		aRes=pCon->_sqlDoQuery(sQuery,iFlags,iLastId,sParams,iParCnt,pCon->_pPluginData,&pCon->sErrMld);
		stlMutexUnlock(&pCon->conMux);

		iTime=stlMsTimer(iTime);
		// Als query langer dan 500ms duurde melding genereren
		if (iTime>500)	printf("Warning SQL (ext) Query %d ms %s\n",iTime,sQshort->sBuf);

		stlFree(sQshort);
		return aRes;
	}

	pCon->iDbFlags = (pCon->iDbFlags & 0xFF00) | iFlags;

	//pCon->iDbFlags |= _DbFlgPrintCmd_;

	aRes=_sqlDoQuery(pCon,iLastId);
	if (pCon->iDbFlags & _dbFlgReconnect_ )
	{
		if (_sqlConnect(pCon)==0)							// Reconnect met database
			pCon->iDbFlags |= _DbFlgRetryCmd_;	// Als gelukt opnieuw uitvoeren command
	}

	if (pCon->iDbFlags & _DbFlgRetryCmd_)
	{
		pCon->iDbFlags &= ~_DbFlgRetryCmd_;
		stlFree(aRes);
		pCon->iParCnt=iParCnt;
		pCon->sQuery =sQuery;
		aRes=_sqlDoQuery(pCon,iLastId);					// Re execute last commando
	}

	if (pCon->iDbFlags & _DbFlgSelectDb_){	// Database is weg geweest resent use <database>
		if ((pCon->iDbType == _DbTypeMySql_)&&(pCon->sDbInfo))
		{
			stlFree(aRes);
			aRes=stlGetStr(pCon->sDbInfo,2,0,0);
			pCon->iParCnt=0;
			pCon->sQuery =stlSetStf("use %s",aRes->sBuf);
			stlFree(aRes);
			aRes=_sqlDoQuery(pCon,NULL);
			stlFree(aRes);
			stlFree(pCon->sQuery);
			pCon->iParCnt=iParCnt;
			pCon->sQuery =sQuery;
			aRes=_sqlDoQuery(pCon,iLastId);				// Re execute last commando
		}
	}
	pCon->iDbFlags &= ~_dbFlgReconnect_;
	pCon->iDbFlags &= ~_DbFlgRetryCmd_;
	pCon->iDbFlags &= ~_DbFlgSelectDb_;

	stlMutexUnlock(&pCon->conMux);
	stlFree(sQshort);
	return aRes;
}


STP stlSqlQuery(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt)
{
	return stlSqlConQuery(sQuery,iFlags,iLastId,sParams,iParCnt,0);
}

STP stlSqlQfast(STP sQuery)
{
	return stlSqlConQuery(sQuery,0,NULL,NULL,0,0);
}

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
STP stlSqlQueryf(const char *sCommand,...)
{
	STP sCmd,sAns;
	va_list ap;
	va_start(ap,sCommand);
	sCmd=stlSetSta(sCommand,ap);
	va_end(ap);
	sAns=stlSqlQuery(sCmd,0,NULL,NULL,0);
	stlFree(sCmd);
	return sAns;
}
/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
STP stlSqlParQueryf(STP *sParams,int iParCnt,const char *sCommand,...)
{
	STP sCmd,sAns;
	va_list ap;
	va_start(ap,sCommand);
	sCmd=stlSetSta(sCommand,ap);
	va_end(ap);
	sAns=stlSqlQuery(sCmd,0,NULL,sParams,iParCnt);
	stlFree(sCmd);
	return sAns;
}

int stlSqlConCommandf(int iCon,const char *sCommand,...)
{
	int iAns=-1;
	STP sCmd,sAns;
	va_list ap;
	va_start(ap,sCommand);
	sCmd=stlSetSta(sCommand,ap);
	va_end(ap);
	sAns=stlSqlConQuery(sCmd,0,NULL,NULL,0,iCon);
	if (sAns) iAns=atoi(sAns->sBuf);
	stlFree(sCmd);
	stlFree(sAns);
	return iAns;
}

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlCommandf(const char *sCommand,...)
{
	int iAns=-1;
	STP sCmd,sAns;
	va_list ap;
	va_start(ap,sCommand);
	sCmd=stlSetSta(sCommand,ap);
	va_end(ap);
	sAns=stlSqlQuery(sCmd,0,NULL,NULL,0);
	if (sAns) iAns=atoi(sAns->sBuf);
	stlFree(sCmd);
	stlFree(sAns);
	return iAns;
}

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlParCommandf(STP *sParams,int iParCnt,const char *sCommand,...)
{
	int iAns=-1;
	STP sCmd,sAns;
	va_list ap;
	va_start(ap,sCommand);
	sCmd=stlSetSta(sCommand,ap);
	va_end(ap);
	sAns=stlSqlQuery(sCmd,0,NULL,sParams,iParCnt);
	if (sAns) iAns=atoi(sAns->sBuf);
	stlFree(sCmd);
	stlFree(sAns);
	return iAns;
}

/* Voer SQL commando uit op database met printf parameters en geef resultaat terug
*/
int stlSqlCommand(const char *sCommand)
{
	int iAns=-1;
	STP sCmd=stlSetSt(sCommand);
	STP sAns=stlSqlQuery(sCmd,0,NULL,NULL,0);
	if (sAns) iAns=atoi(sAns->sBuf);
	stlFree(sAns);
	stlFree(sCmd);
	return iAns;
}
