#include "stl_str.h"
#include "stl_tcp.h"

using namespace ansStl;

class stlTftp
{
public:
	enum transMode{TFTP_ascii = 0,TFTP_binary};
private:
	void initVars(){iTransMode = TFTP_binary;}
	cST sHost;
	int iTransMode;
public:
	stlTftp(const char *host);
	STP get(const char *fn);
	int put(const char *fn,STP data);
	int put(const char *fn,cST &data);
	void mode(transMode newMode);
};
