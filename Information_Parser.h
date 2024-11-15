#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INFP_NULL		0

typedef struct TAGInformation *pTAGInformation;
struct TAGInformation {
	unsigned char *pBuffer;
	int iSize;
	pTAGInformation pNext;
	pTAGInformation pMemChain;
};

class Information_Parser
{
public:
	Information_Parser(void);
	~Information_Parser(void);

	int m_iLastError;
	char *m_szLastError;

	bool OpenInfoFile(const char *szFileName);
	void CloseFile(void);
	void DeleteInfo(void);

	pTAGInformation GetTag(const char *szTagName, pTAGInformation pFrom = INFP_NULL, int *piNum = INFP_NULL, int *pError = INFP_NULL, const char *szDebugTrace = INFP_NULL);
	char *GetProperty(pTAGInformation pTag, char *szPropName);
	static void RemoveNewLine(char *szStr, int iSize);
	static int GetSplitWord(const char *szFrom, int iFromSize, char *szTo, int iToSize, char cSplit, int iOrder);

private:
	unsigned char *m_pBuffer;
	int m_iBufferSize;
	pTAGInformation m_pAllocatedTag;

	bool GetTagString(char *pBuf, int iStartPosition, int iSize, char *pString, int iStringSize, int *piType, int *piStPos, int *piEdPos);
	void SetError(int iErr, char *szErr, int iErrLen = 0);
};
