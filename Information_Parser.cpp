#include "Information_Parser.h"
#include "common_inc.h"

#define DARK_NEW new

Information_Parser::Information_Parser(void)
: m_pBuffer(INFP_NULL)
, m_iBufferSize(0)
, m_iLastError(0)
, m_pAllocatedTag(INFP_NULL)
, m_szLastError(INFP_NULL)
{
}

Information_Parser::~Information_Parser(void)
{
	if( m_pBuffer != INFP_NULL ) delete [] m_pBuffer;
	m_pBuffer = INFP_NULL;
	if (m_szLastError != INFP_NULL) delete[] m_szLastError;

	DeleteInfo();
}

void Information_Parser::DeleteInfo()		
{
	pTAGInformation pT;	

	while( m_pAllocatedTag != INFP_NULL )
	{
		pT = m_pAllocatedTag;
		m_pAllocatedTag = m_pAllocatedTag->pMemChain;

		if( pT->pBuffer != INFP_NULL ) delete [] pT->pBuffer;
		delete pT;
	}
}

bool Information_Parser::OpenInfoFile(const char *szFileName)
{
	int iSize;
	unsigned char cChar;
	FILE *fp = INFP_NULL;
	DARKPIF_fopen(&fp, szFileName, "r");
	if( fp == INFP_NULL ) {
		m_iLastError = 1;
		return false;
	}

	iSize = 0;
	fseek( fp, 0, SEEK_END );
	iSize = ftell(fp);
	fseek( fp, 0, SEEK_SET );
	//while( fgetc(fp) != EOF ) iSize++;

	if( iSize <= 0 ) {
		fclose(fp);
		m_iLastError = 2;
		return false;
	}

	if( m_pBuffer != INFP_NULL ) delete [] m_pBuffer;
	m_pBuffer = DARK_NEW unsigned char[iSize];

	if( m_pBuffer == INFP_NULL ) {
		fclose(fp);
		m_iLastError = 3;
		return false;
	}

	fclose(fp);

	fp = INFP_NULL;
	DARKPIF_fopen(&fp, szFileName, "r");

	m_iBufferSize = 0;

	bool bComment;
	bool bStart = false;

	m_iBufferSize = 0;
	bComment = false;
	while( fread( &cChar, 1, 1, fp ) > 0 )
	{
		if( bStart == false )
		{
			if( !(cChar == '\t' || cChar == '\n' || cChar == '\r' || cChar == '\n') ) {
				if( cChar == '#' ) bComment = true;
				else {
					m_pBuffer[m_iBufferSize] = cChar;
					m_iBufferSize++;
					bComment = false;
				}
				bStart = true;
			}
		}
		else {
			if( cChar == '\n' || cChar == '\r' ) {
				m_pBuffer[m_iBufferSize] = '\n';
				m_iBufferSize++;
				bStart = false;
			}
			else {
				if( bComment == false ) {
					m_pBuffer[m_iBufferSize] = cChar;
					m_iBufferSize++;
				}
			}
		}

		if( m_iBufferSize > iSize ) break;
	}

	fclose(fp);
	return true;
}

void Information_Parser::CloseFile()
{
	if( m_pBuffer != INFP_NULL ) delete [] m_pBuffer;
	m_pBuffer = INFP_NULL;
}

pTAGInformation Information_Parser::GetTag(const char *szTagName, pTAGInformation pFrom, int *piNum, int *pError, const char *szDebugTrace)
{
	pTAGInformation pRes, pF, pR1, pR2;
	int i, iRes;// i2;

	if( piNum != INFP_NULL ) *piNum = 0;
	if (pError != INFP_NULL) *pError = 0;

	if( pFrom == INFP_NULL ) {
		if( m_pBuffer == INFP_NULL ) return INFP_NULL;
		pF = DARK_NEW TAGInformation;
		pF->pBuffer = m_pBuffer;
		pF->iSize = m_iBufferSize;
		pF->pNext = INFP_NULL;
	}
	else pF = pFrom;

	pRes = INFP_NULL;
	pR1 = INFP_NULL;

	char szTag[255];
	bool bContBegin;
	int iTagType, iTagStPos, iTagEdPos;
	int iRealStPos, iRealEdPos, iTagLevel;
	bool bComplete;

	//////////////////////////////////////////////////////////////////////////
	// Debugging
	char *pTraceStack = INFP_NULL;
	int iTrackOffset = 0;
	if (szDebugTrace != INFP_NULL) {
		pTraceStack = DARK_NEW char[pF->iSize];
		if (pTraceStack == INFP_NULL) {
			SetError(100, (char *)("Not enough memory"));
			if (pError != INFP_NULL) *pError = -100;
			return NULL;
		}
		memset(pTraceStack, 0, pF->iSize);
	}
	//////////////////////////////////////////////////////////////////////////

	bContBegin = false;

	iRes = 0;

	i = 0;
	iTagLevel = 0;
	bComplete = false;
	while( i < pF->iSize ) {
		memset( szTag, 0, 255 );
		if (GetTagString((char *)pF->pBuffer, i, pF->iSize, szTag, 255, &iTagType, &iTagStPos, &iTagEdPos) == false) {
			break;
		}
		else {
			if( bContBegin == false ) {
				if( iTagType == 0 ) { //���� �ΰ�..
					if( strcmp( szTagName, szTag ) == 0 ) { //ã�� �±װ� �³�?
						iRealStPos = iTagEdPos + 1;
						bContBegin = true;
						iTagLevel = 0;
						if (pError != INFP_NULL) *pError = 10000;
					}
				}
			}
			else {
				if (iTagType == 0) {
					iTagLevel++;
					if (pError != INFP_NULL) *pError += 1;
				}
				else { //���� �±�
					if( iTagLevel == 0 ) {
						if( strcmp( szTagName, szTag ) == 0 ) { //ã�� �±װ� �³�?
							iRealEdPos = iTagStPos - 1;
							bComplete = true;
							bContBegin = false;
							if (pError != INFP_NULL) *pError += 10000;
						}
					}
					else {
						iTagLevel--;
						if (pError != INFP_NULL) *pError -= 1;
					}
				}
			}			
			i = iTagEdPos + 1;

			//////////////////////////////////////////////////////////////////////////
			// Debugging
			/*
			if (szDebugTrace != INFP_NULL) {
				FILE *fp = fopen(szDebugTrace, "a+");
				for (i2 = 0; i2 < iTagLevel + iTagType; i2++) fprintf(fp, "\t");
				fprintf(fp, "%d:%d:%s\r\n", iTagLevel + iTagType, iTagType, szTag);
				fclose(fp);

				if (iTagType == 0) {
					iTrackOffset += (int) strlen(pTraceStack + iTrackOffset) + 1;
					strcpy(pTraceStack + iTrackOffset, szTag);
				}
				else {
					if (strcmp(pTraceStack + iTrackOffset, szTag) == 0) {
						memset(pTraceStack + iTrackOffset, 0, strlen(szTag));
						iTrackOffset--;
						while (iTrackOffset > 0) {
							iTrackOffset--;
							if (pTraceStack[iTrackOffset] == 0) {
								iTrackOffset++;
								break;
							}
						}
					}
					else {
						bComplete = false;
						pRes = NULL;
						char *szT = DARK_NEW char[1024];
#ifdef DARK_SNPRINTF
#ifdef _MSC_VER
						DARK_SNPRINTF(szT, 1024, _TRUNCATE, "Incorrect tag stack, CurrentTag: %s, ClosedTag: %s", pTraceStack + iTrackOffset, szTag);
#else
						sprintf(szT, "Incorrect tag stack, CurrentTag: %s, ClosedTag: %s", pTraceStack + iTrackOffset, szTag);
#endif
#else
						sprintf(szT, "Incorrect tag stack, CurrentTag: %s, ClosedTag: %s", pTraceStack + iTrackOffset, szTag);
#endif
						SetError(1001, szT);
						if (pError != INFP_NULL) *pError = -1001;
						delete[] szT;
						break;
					}
				}
			}
			*/
			//////////////////////////////////////////////////////////////////////////
		}

		if( bComplete == true ) {
			pR2 = DARK_NEW TAGInformation;
			memset( pR2, 0, sizeof(TAGInformation) );
			if( pR1 == INFP_NULL ) {
				pRes = pR2;
				pR1 = pR2;
			}
			else {
				pR1->pNext = pR2;
				pR1->pMemChain = pR2;
				pR1 = pR2;
			}
			
			
			iRealEdPos++;//��ĭ �� ���ܾ� ��

			//���� �ٹٲ�, ��, ����, ����
			while( iRealStPos < iRealEdPos) { 
				if( pF->pBuffer[iRealStPos] == '\r' ) iRealStPos++;
				else if( pF->pBuffer[iRealStPos] == '\n' ) iRealStPos++;
				else if( pF->pBuffer[iRealStPos] == '\t' ) iRealStPos++;
				else if( pF->pBuffer[iRealStPos] == ' ' ) iRealStPos++;
				else break;
			} 
			//���� �ٹٲ� ����

			while(iRealEdPos > iRealStPos) { 
				//printf("%c ", pF->pBuffer[iRealEdPos]);
				if( pF->pBuffer[iRealEdPos] == '\r' ) iRealEdPos--;
				else if( pF->pBuffer[iRealEdPos] == '\n' ) iRealEdPos--;
				else break;
			}

			pR2->iSize = iRealEdPos - iRealStPos;
			pR2->pBuffer = DARK_NEW unsigned char[pR2->iSize + 1];
			memcpy( pR2->pBuffer, pF->pBuffer + iRealStPos, pR2->iSize );
			pR2->pBuffer[pR2->iSize] = 0;						

			iRes++;
			bComplete = false;
		}

	} //End of While

	if( pFrom == INFP_NULL ) delete pF;

	if( pRes != INFP_NULL ) {
		if( m_pAllocatedTag == INFP_NULL ) m_pAllocatedTag = pRes;
		else {
			pTAGInformation pT = m_pAllocatedTag;
			while( pT->pMemChain != INFP_NULL ) pT = pT->pMemChain;
			pT->pMemChain = pRes;
		}
	}

	if( piNum != INFP_NULL ) *piNum = iRes;

	if (pTraceStack != INFP_NULL) delete[] pTraceStack;
	return pRes;
}

char *Information_Parser::GetProperty(pTAGInformation pTag, char *szPropName)
{
	int i, j, iLen;
	unsigned char byT[1024];

	if( pTag == INFP_NULL ) return INFP_NULL;
	memset( byT, 0, 1024 );
	
	iLen = (int)strlen(szPropName);
	j = 0;
	for( i = 0; i < pTag->iSize; i++ )
	{
		if( pTag->pBuffer[i] == '\n' ) {

			if( strncmp( (char *) byT, szPropName, iLen ) == 0 ) {
				if( byT[iLen] == ':' )
				{
					int iOff = iLen + 1;
					while( iOff < j && byT[iOff] == ' ' ) iOff++;

					pTAGInformation pT = DARK_NEW TAGInformation;
					memset( pT, 0, sizeof(TAGInformation) );

					pT->iSize = j - iOff + 1;
					pT->pBuffer = DARK_NEW unsigned char[pT->iSize];
					memset( pT->pBuffer, 0, pT->iSize );
					memcpy( pT->pBuffer, byT + iOff, pT->iSize - 1 );
					//memcpy( pT->pBuffer, byT, j );

					if( m_pAllocatedTag == INFP_NULL ) m_pAllocatedTag = pT;
					else {
						pTAGInformation pT2 = m_pAllocatedTag;
						while( pT2->pMemChain != INFP_NULL ) pT2 = pT2->pMemChain;
						pT2->pMemChain = pT;
					}				

					return (char *) pT->pBuffer;
				}
			}

			memset( byT, 0, 1024 );
			j = 0;
		}
		else {
			byT[j] = pTag->pBuffer[i];
			j++;
			if( j > 1024 ) j--;
		}
	}

	return INFP_NULL;
}

void Information_Parser::RemoveNewLine(char *szStr, int iSize)
{
	int i, j;
	i = j = 0;
	while( i < iSize )
	{
		if( szStr[i] == '\n' ) {
			memcpy( szStr + i, szStr + i + 1, iSize - i - 1);
			j++;
			memset( szStr + (iSize - j), 0, j );
		}
		else i++;
	}
}

bool Information_Parser::GetTagString(char *pBuf, int iStartPosition, int iSize, char *pString, int iStringSize, int *piType, int *piStPos, int *piEdPos)
{
	bool bTagBegin, bRes;
	int iRes, i, Type;
	int iStPos, iEdPos;
	iRes = 0;
	Type = 0;

	bRes = false;

	bTagBegin = false;

	for( i = iStartPosition; i < iSize; i++ )
	{
		if( bTagBegin == false ) {
			if( pBuf[i] == '<' ) {
				if( i < iSize - 1 ) {
					iStPos = i;
					if( pBuf[i + 1] == '/' ) {
						Type = 1;
						i++;
					}
					else Type = 0;

					bTagBegin = true;
				}
			}
		}
		else {
			if( pBuf[i] == '>' ) {
				iEdPos = i;
				bRes = true;				
				break;
			}
			else {
				pString[iRes] = pBuf[i];
				iRes++;
				if (iRes >= iStringSize) break;
			}
		}
	}

	if( bRes == true ) {
		*piType = Type;
		*piStPos = iStPos;
		*piEdPos = iEdPos;
	}
	else {
		if (bTagBegin == true) {
			SetError(1000, pString, iRes - 1);
		}
	}

	return bRes;
}

int Information_Parser::GetSplitWord(const char *szFrom, int iFromSize, char *szTo, int iToSize, char cSplit, int iOrder)
{
	int iOffset;
	int iCOrder;
	int iResSize;
	bool bIsEnd;

	iOffset = 0;
	iCOrder = 0;
	iResSize = 0;
	bIsEnd = true;

	while (iOffset < iFromSize)
	{
		if (szFrom[iOffset] == 0) {
			break;
		}

		if (iCOrder == iOrder)
		{
			if (szFrom[iOffset] == cSplit) {
				bIsEnd = false;
				break;
			}
			else {
				szTo[iResSize++] = szFrom[iOffset];
				if (iResSize >= iToSize - 1) {
					bIsEnd = false;
					break;
				}
			}
		}
		else {
			if (szFrom[iOffset] == cSplit) iCOrder++;
		}

		iOffset++;
	}

	szTo[iResSize] = 0;

	if (bIsEnd && iResSize <= 0) return -1;
	return iResSize;
}

void Information_Parser::SetError(int iErr, char *szErr, int iErrLen /* = 0 */)
{
	int iLen;

	m_iLastError = iErr;

	if (iErrLen <= 0) iLen = (int)strlen(szErr);
	else iLen = iErrLen;

	if (m_szLastError != INFP_NULL) delete[] m_szLastError;
	m_szLastError = INFP_NULL;

	if (iLen <= 0) return;

	m_szLastError = DARK_NEW char[iLen + 1];
	if (m_szLastError != INFP_NULL) {
		m_szLastError[iLen] = 0;
		memcpy(m_szLastError, szErr, iLen);
	}
}

