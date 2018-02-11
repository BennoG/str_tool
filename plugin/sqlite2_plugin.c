#include <stl_str.h>
#include <stl_sql.h>

#include "sqlite2_plugin.h"

#include "sqlite.h"

#ifdef _WIN32
#  pragma comment(lib, "sqlite.lib")
#  pragma comment(lib, "libStl.lib")
#endif

struct sqliteConData
{
	sqlite *db; 
	STP sDbFileName;
	stlMutex mux;
};

/* sqlite error melding to stl_str library compatible format converter
*/
static void errTransport(char **errmsg,STP *pErrorMeld)
{
	if ((errmsg)&&(*errmsg))
	{
		printf("Sqlite error %s\n",*errmsg);
		if (pErrorMeld)
		{
			if (*pErrorMeld) stlFree(*pErrorMeld);
			*pErrorMeld = stlSetSt(*errmsg);
		}
		sqlite_freemem(*errmsg);
		*errmsg = NULL;
	}
}


static STP sqliteDoQuery(STP sQuery,int iFlags,int *iLastId,STP *sParams,int iParCnt,void *pPluginData,STP *pErrorMeld)
{
	int rc,iRows=0,iCols=0,i,j,nri;
	STP sCmd,sRes=NULL;
	char *errmsg = NULL;
	char **sqlRes = NULL; 
	struct sqliteConData *sd=pPluginData;
	if (sd==NULL) return NULL;
	if (sd->sDbFileName==NULL) return NULL;
	if (sd->db == NULL) sd->db = sqlite_open(sd->sDbFileName->sBuf,0,&errmsg);	// open database if not open
	errTransport(&errmsg,pErrorMeld);
	if (sd->db == NULL) return NULL;

	sCmd = stlCopy(sQuery);
	nri = stlCount(sCmd,'?');
	for (i=0;i<iParCnt;i++)
	{
		if (i>nri) break;		// geen ? velden meer in command string
		// TODO hier nog vervangen door sParms data
	}
	if (iFlags & _DbFlgPrintCmd_) printf("CMD: %s\n",sCmd->sBuf);
	rc = sqlite_get_table(sd->db,sCmd->sBuf,&sqlRes,&iRows,&iCols,&errmsg);	// get result from database
	errTransport(&errmsg,pErrorMeld);
	if(rc==SQLITE_OK)
	{
		sRes=stlSetSt("");
		for (i=0;i<iRows;i++)
		{
			if (sRes->iLen) stlAppendCh(sRes,_D1);
			if ((i==0)&&((iFlags & _DbFlgDoHeader_)==0)) continue;		// do not use headers
			for (j=0;j<iCols;j++)
			{
				char *p = sqlRes[i*iCols + j];
				if (j) stlAppendCh(sRes,_D2);
				if (p) stlAppendSt(sRes,p  );
			}
		}
	}
	if (sqlRes) sqlite_free_table(sqlRes);
	sqlite_close(sd->db);
	sd->db = NULL;
	stlFree(sCmd);
	return sRes;
}


/* register sqlite plug in to SQL library's
*/
int stlSqlite2PluginAdd(int iConNo,char *sDbFileName)
{
	char *errmsg=NULL;
	struct sqliteConData *sd;
	sd = malloc(sizeof(struct sqliteConData));
	if (sd==NULL) return -1;
	memset(sd,0,sizeof(sd));
	stlMutexInit(&sd->mux);
	sd->sDbFileName = stlSetSt(sDbFileName);
	return sqlAddEngine(iConNo,sd,sqliteDoQuery);
}

