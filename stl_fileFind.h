#include "stl_str.h"

#ifdef  __cplusplus
#  include <vector>
#endif

#ifdef _WIN32

#endif
#if defined(__linux__) || defined(__APPLE__)
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
#if defined(__linux__) || defined(__APPLE__)
	DIR *pDIR;
	STP pad,fzk;
#endif
};

int stlFindFirst(const char *path,int flags,struct stlFind_t *buf);
int stlFindNext(struct stlFind_t *buf);
int stlFindClose(struct stlFind_t *buf);

// Get al files in specific DIR (sorted)
STP stlGetFiles(const char *zkp);
// get al files in specific DIR including path (sorted)
STP stlGetFilesDir(const char *zkp);

#ifdef  __cplusplus
}
#endif


#ifdef  __cplusplus
std::vector<ansStl::cST> stlGetFilesAr(const char *zkp);
std::vector<ansStl::cST> stlGetFilesDirAr(const char *zkp);
#endif
