
#include <stl_str.h>
#include <stl_sql.h>

#include "sqlite2_plugin.h"

#ifdef _WIN32
#  pragma comment(lib, "sqlite.lib")
#  pragma comment(lib, "libStl.lib")
#endif

int main()
{
	STP sCmd,sAns;
	stlSqlite2PluginAdd(0,"C:\\prog\\sqlite_gui\\sms.db");
	sCmd=stlSetStf("SELECT * FROM EMAIL");
	sAns=stlSqlQuery(sCmd,_DbFlgDoHeader_,NULL,NULL,0);
	return 0;
}