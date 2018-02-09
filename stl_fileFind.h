#include "stl_str.h"

#ifdef _WIN32

#endif
#ifdef __linux__
#  include <dirent.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif


#define _stlFindDirs    1
#define _stlFindNorm    2
#define _stlFindHid     4
#define _stlFindAll     (_stlFindDirs | _stlFindNorm |_stlFindHid)

struct stlFind_t {
	int Flgs;
	long int size;
	char name[256];
#ifdef _WIN32
	// Internal OS dependent data
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
#endif
#ifdef __linux__
	DIR *pDIR;
	STP pad,fzk;
#endif
};

int stlFindFirst(const char *path,int flags,struct stlFind_t *buf);
int stlFindNext(struct stlFind_t *buf);
int stlFindClose(struct stlFind_t *buf);

// Get al files in specific DIR (sorted)
STP stlGetFiles(const char *zkp);

#ifdef  __cplusplus
}
#endif
