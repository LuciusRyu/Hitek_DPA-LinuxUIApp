#include "common_inc.h"

//#define KEEPALIVED_CONF_FILE    "/etc/keepalived/keepalived.conf"
#define PR_ERR      printf

bool DARKPIF_fopen(FILE **fp, const char *szFilename, const char *szOpt) {
	*fp = fopen(szFilename, szOpt);
	if (*fp == NULL) return false;
	return true;
}

bool GCMN_setup_vrrp(const char *szFile, bool bUse, bool bIsMaster, int iID, int iPriority, const char *szInf, const char *szIP) 
{
    FILE *fp = NULL;
    if (!DARKPIF_fopen(&fp, szFile, "w+")) {
        PR_ERR("ERROR: File open failed: [%s]\n", szFile);
        return false;
    }

    fprintf(fp, "#!/bin/bash\n");
    if (bUse) {        
        fprintf(fp, "DARK_VRRP_USE=1\n");
        fprintf(fp, "DARK_VRRP_MASTER=%d\n", bIsMaster ? 1 : 0);
        fprintf(fp, "DARK_VRRP_INF=\"%s\"\n", szInf);
        fprintf(fp, "DARK_VRRP_ROUTER_ID=%d\n", iID);
        fprintf(fp, "DARK_VRRP_PRIORITY=%d\n", iPriority);
        fprintf(fp, "DARK_VRRP_SVC_IP=\"%s\"\n", szIP);
    }
    else {
        fprintf(fp, "DARK_VRRP_USE=0\n");
    }

    /*
    if (bUse) {        
        fprintf(fp, "vrrp_instance HDPA {\n");
        fprintf(fp, "\tstate %s\n", bIsMaster ? "MASTER" : "BACKUP");
        fprintf(fp, "\tinterface %s\n", szInf);
        fprintf(fp, "\tvirtual_router_id %d\n", iID);
        fprintf(fp, "\tpriority %d\n", iPriority);
        fprintf(fp, "\tadvert_int 3\n");
        fprintf(fp, "\tauthentication {\n");
        fprintf(fp, "\t\tauth_type PASS\n");
        fprintf(fp, "\t\tauth_pass 240311\n");
        fprintf(fp, "\t}\n");
        fprintf(fp, "\tvirtual_ipaddress {\n");
        fprintf(fp, "\t\t%s\n", szIP);
        fprintf(fp, "\t}\n");
        fprintf(fp, "}\n");
    }
    else {
        fprintf(fp, "\n");
    }
    */
    fclose(fp);
    return true;
}

int32_t GCMN_GetSeperateWordWithQuotes(const char *szSeperater, char *pSource, int32_t iSourceSize, char **OUT_ppszRes, int32_t *OUT_piLen, int32_t iOutSize)
{
	int32_t iRes, iSepLen, iOffset, i;
	bool bRes;
	bool bDoubleQuote, bQuote;
	if (iSourceSize <= 0) return 0;
	if (iOutSize < 2) return 0;

	iSepLen = strlen(szSeperater);
	OUT_ppszRes[0] = pSource;
	OUT_piLen[0] = iSourceSize;
	iRes = 1;
	if (iSepLen <= 0) return iRes;

	bDoubleQuote = false;
	bQuote = false;

	iOffset = 0;
	OUT_piLen[0] = 0;
	while (iOffset < iSourceSize) {
		if (!bQuote && pSource[iOffset] == 0x22) { //"
			if (bDoubleQuote) bDoubleQuote = false;
			else {
				bDoubleQuote = true;
				if (OUT_piLen[iRes - 1] == 0) OUT_ppszRes[iRes - 1]++; //최초면 한칸 전진
			}
			iOffset++; //인용문 기호는 무시
		}
		else if (!bDoubleQuote && pSource[iOffset] == 0x27) { //'
			if (bQuote) bQuote = false;
			else {
				bQuote = true;
				if (OUT_piLen[iRes - 1] == 0) OUT_ppszRes[iRes - 1]++; //최초면 한칸 전진
			}
			iOffset++; //인용문 기호는 무시
		}
		else {
			bRes = true;
			if (bDoubleQuote || bQuote) { //인용문 중이면 무시한다.
				bRes = false;
			}
			else {
				if (iSourceSize - iOffset >= iSepLen) {
					for (i = 0; i < iSepLen; i++) {
						if (pSource[iOffset + i] != szSeperater[i]) {
							bRes = false;
							break;
						}
					}
				}
				else bRes = false;
			}

			if (bRes == true) {
				iOffset += iSepLen;
				OUT_ppszRes[iRes] = pSource + iOffset;
				OUT_piLen[iRes] = 0;
				iRes++;
				if (iRes >= iOutSize) {
					OUT_piLen[iRes - 1] = iSourceSize - iOffset;
					return iRes;
				}
			}
			else {
				iOffset++;
				OUT_piLen[iRes - 1]++;
			}
		}
	}

	return iRes;
}

int32_t GCMN_Hex2Int(const char* szHex, bool bCheck0x) {
	int iSt = 0;

	size_t len = strlen(szHex);
	if (bCheck0x) {
		if (len < 2) return atoi(szHex);
		if (!(szHex[0] == '0' && szHex[1] == 'x')) return atoi(szHex);
		iSt = 2;
	}

	char c;
	int32_t res = 0;
	int32_t v = 0;
	for (int32_t i = 0; i + iSt < (int32_t)len; i++) {
		c = szHex[i + iSt];		
		if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
		else v = c - '0'; //if (c >= '0' && c <= '9') v = c - '0';
		res = res * 0x10 + v;
	}

	return res;
}

int32_t GCMN_HexList2Int(char* szHexList, const char* szSeperater, uint32_t nMaxList, int32_t* OUT_pBuf, bool bCheck0x)
{
	char* szTemp = new char[255];
	char** ppRes = new char* [(size_t)(nMaxList + 1)];
	int32_t* piLen = new int32_t[(size_t)(nMaxList + 1)];
	if (szTemp == NULL || ppRes == NULL || piLen == NULL) return 0;

	//printf("HexList2Int: %s\n", szHexList);
	int32_t iRes = GCMN_GetSeperateWordWithQuotes(szSeperater, szHexList, (int32_t)strlen(szHexList), ppRes, piLen, (int32_t)nMaxList);
	if (iRes > 0) {
		for (int32_t i = 0; i < iRes; i++) {
			memset(szTemp, 0, 255);
			memcpy(szTemp, ppRes[i], piLen[i]);
			OUT_pBuf[i] = (int32_t)GCMN_Hex2Int(szTemp, bCheck0x);
			//printf("\tStr=%s, val=0x%02X\n", szTemp, OUT_pBuf[i]);
		}
	}

	delete[] szTemp;
	delete[] ppRes;
	delete[] piLen;
	return iRes;
}



///////////////////////////////////////////////////////////////////////////////////////////
DarkString::DarkString() 
	: m_szString(NULL)
	, m_uiSize(0)
{
}

DarkString::DarkString(const char *szString)
	: DarkString()
{	
	SetString(szString);
}

DarkString::~DarkString()
{
	if (m_szString != NULL) delete[] m_szString;
}

bool DarkString::SetString(const char *szString)
{
	if (szString == NULL) return false;
	if (m_szString != NULL) delete[] m_szString;	
	m_uiSize = (uint32_t)strlen(szString);
	m_szString = new char[m_uiSize + 1];
	if (m_szString == NULL) return false;

	memcpy(m_szString, szString, m_uiSize);
	m_szString[m_uiSize] = 0;

	return true;
}

int32_t DarkString::base64_encode(char *in, int32_t in_size, char *out, int32_t out_size) {
	static const char base64code_ascii[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int32_t i = 0;
	int32_t o = 0;
	int32_t len;

	if (!in || !out) return 0;

	len = (strlen(in) + 2) / 3 * 4;
	if (len > out_size) return 0;

	while (i < in_size - 2 && o + 4 <= out_size)
	{
		out[o++] = base64code_ascii[(in[i] >> 2) & 0x3F];
		out[o++] = base64code_ascii[((in[i] & 0x3) << 4) | ((int)(in[i + 1] & 0xF0) >> 4)];
		out[o++] = base64code_ascii[((in[i + 1] & 0xF) << 2) | ((int)(in[i + 2] & 0xC0) >> 6)];
		out[o++] = base64code_ascii[in[i + 2] & 0x3F];
		i += 3;
	}

	if (i < in_size)
	{
		out[o++] = base64code_ascii[(in[i] >> 2) & 0x3F];
		if (i == (in_size - 1))
		{
			out[o++] = base64code_ascii[((in[i] & 0x3) << 4)];
			out[o++] = '='; // padding
		}
		else
		{
			out[o++] = base64code_ascii[((in[i] & 0x3) << 4) | ((int)(in[i + 1] & 0xF0) >> 4)];
			out[o++] = base64code_ascii[((in[i + 1] & 0xF) << 2)];
		}

		out[o++] = '='; // padding
	}

	return o;
}
