#ifndef _DARK_HITEK_COMMON_INC
#define _DARK_HITEK_COMMON_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "JSON_CPP/json/json.h"
#include "Information_Parser.h"

#define HITEK_UI_VER	"1.0"

#define DARK_MAXPATH    1024
#define DARK_SHORTSTR_S 64
#define DARK_MIDSTR_S   255

enum class DARK_SYS_TYPE {
    NONE = 0,
    MAIN_TX,
    REMOTE,
    INT_CNC
};

inline void DARK_STRCPY(char* pTar, int iTSize, const char* pSrc) {
	int iLen = (int)strlen(pSrc);
	memcpy(pTar, pSrc, iLen);
	pTar[iLen] = 0;
}


#define DARKCRITICAL_TYPE		pthread_mutex_t
#define DARKCRITICAL_INIT(x)	pthread_mutex_init(&x, 0)
#define DARKCRITICAL_DELETE(x)	pthread_mutex_destroy(&x)
#define DARKCRITICAL_LOCK(x)		pthread_mutex_lock(&x)
#define DARKCRITICAL_UNLOCK(x)		pthread_mutex_unlock(&x)
#define DARK_SNPRINTF2 				snprintf
#define DARK_SLEEP					DARK_mSleep

typedef struct StringChain *pStringChain;
struct StringChain {
    int iLen;
    char *szString;
    pStringChain pNext;
};

bool DARKPIF_fopen(FILE **fp, const char *szFilename, const char *szOpt);
bool GCMN_setup_vrrp(const char *szFile, bool bUse, bool bIsMaster, int iID, int iPriority, const char *szInf, const char *szIP);
int32_t GCMN_GetSeperateWordWithQuotes(const char *szSeperater, char *pSource, int32_t iSourceSize, char **OUT_ppszRes, int32_t *OUT_piLen, int32_t iOutSize);
int32_t GCMN_Hex2Int(const char* szHex, bool bCheck0x = true);
int32_t GCMN_HexList2Int(char* szHexList, const char *szSeperater, uint32_t nMaxList, int32_t *OUT_pBuf, bool bCheck0x = true);

inline unsigned int DARKPIF_GetTickCount()
{
	struct timespec ts;
	unsigned int theTick = 0U;
	clock_gettime( CLOCK_MONOTONIC, &ts);
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	return theTick;
}

inline void DARK_mSleep(unsigned int ms) {
	struct timeval tv;
	if( ms == 0 || ms > 3600000 ) return; //1시간 이상은 슬립 할 수 없다.
	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;

	select(0, NULL, NULL, NULL, (struct timeval *)&tv);
}

class DarkString
{
public:
	DarkString();
	DarkString(const char *szString);
	~DarkString();

	bool SetString(const char *szString);

	char *m_szString;
	uint32_t m_uiSize;
};

#endif