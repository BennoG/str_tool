#ifndef _STL_FTP_CLIENT_H_
#define _STL_FTP_CLIENT_H_

#include "stl_tcp.h"
#include "stl_str.h"

#ifdef  __cplusplus

class ftpClient
{
private:
	void initVar(){stc = NULL; sPass = sUser = sHost = sAnswer = NULL; ftpPort = 21; timeoutMs = 10000; loggedIn = false;}
	STP sPass,sUser,sHost,sAnswer;
	stlTcpConn *stc;
	bool loggedIn;
public:
	int timeoutMs;
	int ftpPort;
	ftpClient(const char *host = NULL,const char *user = NULL,const char *pass = NULL,int port = 0);
	~ftpClient();
public:
	void setPort(int iPortNr);
	void setHost(const char *host);
	void setPass(const char *pass);
	void setUser(const char *user);
	int  connect(const char *host = NULL,const char *user = NULL,const char *pass = NULL,int port = 0);
	int  login(const char *user = NULL,const char *pass = NULL);
	int  setPWD(const char *dir);
	int  setCWD(const char *dir){return setPWD(dir);}
	STP  getPWD();
	STP  getList(int iFormat = 0);
	STP  download(const char *fmt,...);
private:
	/* send command to FTP server and wait for its answer
	** return > 0 result code server
	**        < 0 error
	*/
	int command(const char *fmt,...);
	/* get return from ftp server
	** return < 0 error
	**        > ftp code
	**        -5 timeout (connection closed ?)
	*/
	int getAnswer();
	/* enter passive mode on ftp server */
	stlTcpConn *enterPassive();
};

extern "C" {
#endif	// __cplusplus

void testFtpClient();

#ifdef  __cplusplus
}
#endif	// __cplusplus

#endif	// _STL_FTP_CLIENT_H_
