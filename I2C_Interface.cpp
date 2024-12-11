#include "I2C_Interface.h"

#ifndef _MSC_VER
#include <fcntl.h>				
#include <sys/ioctl.h>			
#include <linux/i2c.h>		
#include <linux/i2c-dev.h>		
#endif

#define _DarkLogD	printf
#define _DarkLogE	printf
#define _DarkLogW	printf

I2C_ForEach::I2C_ForEach() {
	m_slaveAddr = 0;
	m_switchLine = 0;
	m_sizes_check = 0;
	m_sizes_init = 0;
	m_sizes_map = 0;
	m_datas_check = NULL;
	m_datas_init = NULL;
	m_datas_map_v = NULL;
	m_datas_map_a = NULL;
	m_cState = 0;
	m_fp = -1;
	m_pParent = NULL;
	m_bUseSwitch = false;
}

I2C_ForEach::~I2C_ForEach() {
	if (m_datas_check != NULL) delete[] m_datas_check;
	if (m_datas_init != NULL) delete[] m_datas_init;
	if (m_datas_map_v != NULL) delete[] m_datas_map_v;
	if (m_datas_map_a != NULL) delete[] m_datas_map_a;
#ifndef _MSC_VER
	if (m_fp >= 0) close(m_fp);
#endif
}

bool I2C_ForEach::SetupData_Check(char* szAddr, char* szData)
{
	int32_t* pBufA = new int32_t[255];
	int32_t* pBufD = new int32_t[255];	
	if (pBufA == NULL || pBufD == NULL) return false;

	int32_t iResA = GCMN_HexList2Int(szAddr, ",", 255, pBufA);
	int32_t iResD = GCMN_HexList2Int(szData, ",", 255, pBufD);	

	bool bRes = false;
	int32_t i;
	if (iResA == iResD) {		
		m_datas_check = new i2c_8bit_pair[iResA];
		if (m_datas_check != NULL) {
			bRes = true;
			m_sizes_check = iResA;
			for (i = 0; i < iResA; i++) {
				m_datas_check[i].wAddr = (uint16_t)pBufA[i];
				m_datas_check[i].cVal = (uint8_t)pBufD[i];
				m_datas_check[i].cData = 0;
			}
		}
	}
	else _DarkLogE("SetupData_Check: Address - data count not match, %d:%d\n", iResA, iResD);

	delete[] pBufA;
	delete[] pBufD;
	_DarkLogD("Slave addr 0x%X (%d) - Check datas\n", m_slaveAddr, m_switchLine);
	{
		char* szT = new char[1024];
		DARK_SNPRINTF2(szT, 1024, "\tAddr:Val =>");
		for (i = 0; i < m_sizes_check; i++) {
			DARK_SNPRINTF2(szT + strlen(szT), 1024 - strlen(szT), " 0x%02X:0x%02X", m_datas_check[i].wAddr, m_datas_check[i].cVal);
		}
		_DarkLogD(szT);
		delete[] szT;
	}
#ifdef _MSC_VER
	m_sizes_check = 0; //그냥 통과시키기 위함
#endif
	return bRes;
}

bool I2C_ForEach::SetupData_Init(char* szAddr, char* szData) 
{
	int32_t* pBufA = new int32_t[255];
	int32_t* pBufD = new int32_t[255];
	if (pBufA == NULL || pBufD == NULL) return false;

	int32_t iResA = GCMN_HexList2Int(szAddr, ",", 255, pBufA);
	int32_t iResD = GCMN_HexList2Int(szData, ",", 255, pBufD);

	int32_t i;
	bool bRes = false;
	if (iResA == iResD) {
		m_datas_init = new i2c_8bit_pair[iResA];
		if (m_datas_init != NULL) {
			bRes = true;
			m_sizes_init = iResA;
			for (i = 0; i < iResA; i++) {
				m_datas_init[i].wAddr = (uint16_t)pBufA[i];
				m_datas_init[i].cData = (uint8_t)pBufD[i];
				m_datas_init[i].cVal = 0;
			}
		}
	}
	else _DarkLogE("SetupData_Init: Address - data count not match, %d:%d", iResA, iResD);

	delete[] pBufA;
	delete[] pBufD;
	_DarkLogD("Slave addr 0x%X (%d) - Init datas", m_slaveAddr, m_switchLine);
	{
		char* szT = new char[1024];
		DARK_SNPRINTF2(szT, 1024, "\tAddr:Val =>");
		for (i = 0; i < m_sizes_init; i++) {
			DARK_SNPRINTF2(szT + strlen(szT), 1024 - strlen(szT), " 0x%02X:0x%02X", m_datas_init[i].wAddr, m_datas_init[i].cData);
		}
		_DarkLogD(szT);
		delete[] szT;
	}
#ifdef _MSC_VER
	m_sizes_init = 0; //그냥 통과시키기 위함
#endif
	return bRes;
}

bool I2C_ForEach::ChMap_Init(uint32_t nCount)
{
	if (nCount < 1) return true;
	m_datas_map_v = new i2c_8bit_pair[nCount];
	m_datas_map_a = new i2c_8bit_pair[nCount];
	if (m_datas_map_v == NULL || m_datas_map_a == NULL) return false;
	m_sizes_map = (int32_t)nCount;
	for (int32_t i = 0; i < m_sizes_map; i++) {
		m_datas_map_v[i].wAddr = 0;
		m_datas_map_v[i].cData = 0;
		m_datas_map_v[i].cVal = 0xFF;
		m_datas_map_a[i].wAddr = 0;
		m_datas_map_a[i].cData = 0;
		m_datas_map_a[i].cVal = 0xFF;
	}
	return true;
}

bool I2C_ForEach::ChMap_Setup(uint32_t nCh, uint8_t cVolAddr, uint8_t cAttrAddr)
{
	if (m_sizes_map < 1) return false;
	if (nCh > 255) return false;
	bool bRes = false;
	for (int32_t i = 0; i < m_sizes_map; i++) {
		if (m_datas_map_v[i].cVal == 0xFF) {
			bRes = true;
			m_datas_map_v[i].wAddr = (uint16_t)cVolAddr;
			m_datas_map_v[i].cVal = (uint8_t)nCh;
			m_datas_map_a[i].wAddr = (uint16_t)cAttrAddr;
			m_datas_map_a[i].cVal = (uint8_t)nCh;
			break;
		}
	}
	return bRes;
}

int8_t I2C_ForEach::Open(I2C_Interface *pParent, const char* szDevName)
{
	m_pParent = pParent;
	if (m_cState != 0) return -1;

	if (m_bUseSwitch) {
		m_fp = pParent->m_fp;
		m_cState = 1;
		return 0;
	}

#ifndef _MSC_VER
	if ((m_fp = open(szDevName, O_RDWR)) < 0) {
		_DarkLogE("File open failed: %s\n", szDevName);
		return -2;
	}	
	int32_t addr = m_slaveAddr;
	if (ioctl(m_fp, I2C_SLAVE, addr) < 0)
	{
		_DarkLogE("Failed to acquire bus access and/or talk to slave.\n");
		close(m_fp);
		m_fp = -1;
		return -3;
	}
#endif
	m_cState = 1;
	return 0;
}

bool I2C_ForEach::Read8bit(i2c_8bit_pair* pPair) {
	if (m_cState < 1) return false;
	if (m_bUseSwitch) return m_pParent->Read8bit(this, pPair);
#ifndef _MSC_VER
	uint8_t addr = (uint8_t)pPair->wAddr;
	uint8_t data = 0;

	struct i2c_msg msg[2];
	struct i2c_rdwr_ioctl_data idata;
	msg[0].addr = (__u16)m_slaveAddr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;
	msg[1].addr = (__u16)m_slaveAddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &data;

	idata.msgs = msg;
	idata.nmsgs = 2;

	if (ioctl(m_fp, I2C_RDWR, &idata) < 0) {
		return false;
	}

	pPair->cData = data;
#endif
	return true;
}

bool I2C_ForEach::Write8bit(i2c_8bit_pair* pPair) {
	if (m_cState < 1) return false;
	if (m_bUseSwitch) return m_pParent->Write8bit(this, pPair);
#ifndef _MSC_VER
	uint8_t buf[2];
	buf[0] = (uint8_t)pPair->wAddr;
	buf[1] = pPair->cData;
	if (write(m_fp, buf, 2) != 2) {
		return false;
	}
#endif
	return true;
}

int8_t I2C_ForEach::CheckAndInit()
{
	if (m_cState < 1) return -1;

	uint32_t i;
	if (m_sizes_check > 0) {
		for (i = 0; i < m_sizes_check; i++) {
			if (m_datas_check[i].wAddr > 0xFF00) { //0xFF00 이상은 Sleep, 10ms 단위
				DARK_SLEEP((m_datas_check[i].wAddr & 0x00FF) * 10);
			}
			else {
				if (!Read8bit(&m_datas_check[i])) {
					_DarkLogE("I2C Check failed, slave 0x%02X, addr=0x%02X, Data read failed\n", m_slaveAddr, m_datas_check[i].wAddr);
					return -2;
				}
				if (m_datas_check[i].cData != m_datas_check[i].cVal) {
					_DarkLogE("I2C Check failed, slave 0x%02X, addr=0x%02X, Data not matched, 0x%02X : 0x%02X\n", m_slaveAddr, m_datas_check[i].wAddr, m_datas_check[i].cData, m_datas_check[i].cVal);
					return -3;
				}
			}
		}
		_DarkLogD("Slave 0x%02X, Check data verification complete\n", m_slaveAddr);
	}
	if (m_sizes_init > 0) {
		for (i = 0; i < m_sizes_init; i++) {
			if (m_datas_init[i].wAddr > 0xFF00) { //0xFF00 이상은 Sleep, 10ms 단위
				DARK_SLEEP((m_datas_init[i].wAddr & 0x00FF) * 10);
			}
			else {
				if (!Write8bit(&m_datas_init[i])) {
					_DarkLogE("I2C Init failed, slave 0x%02X, addr=0x%02X, Data write failed\n", m_slaveAddr, m_datas_init[i].wAddr);
					return -4;
				}
			}
		}
		_DarkLogD("Slave 0x%02X, Init data %d write complete\n", m_slaveAddr, m_sizes_init);
	}
	return 0;
}

int32_t I2C_ForEach::ReadVolumes(uint32_t nBufSize, int32_t* OUT_pCh, int32_t* OUT_pVol) 
{
	int32_t res = 0;
	uint32_t i;
	for (i = 0; i < m_sizes_map; i++) {
		if (m_datas_map_v[i].wAddr == 0) continue;
		if (!Read8bit(&m_datas_map_v[i])) {
			_DarkLogW("I2C Read failed, slave 0x%02X, vol addr=0x%02X\n", m_slaveAddr, m_datas_map_v[i].wAddr);
			return -1;
		}
	}
	for (i = 0; i < m_sizes_map && i < nBufSize; i++) {
		OUT_pCh[i] = m_datas_map_v[i].cVal;
		OUT_pVol[i] = m_datas_map_v[i].cData;
		res++;
	}
	return res;
}

int32_t I2C_ForEach::ReadAttribute(uint32_t nBufSize, int32_t* OUT_pCh, int32_t* OUT_pAttr) 
{
	int32_t res = 0;
	uint32_t i;
	for (i = 0; i < m_sizes_map; i++) {
		if (m_datas_map_a[i].wAddr == 0) continue;
		if (!Read8bit(&m_datas_map_a[i])) {
			_DarkLogW("I2C Read failed, slave 0x%02X, attr addr=0x%02X\n", m_slaveAddr, m_datas_map_a[i].wAddr);
			return -1;
		}
	}
	for (i = 0; i < m_sizes_map && i < nBufSize; i++) {
		OUT_pCh[i] = m_datas_map_a[i].cVal;
		OUT_pAttr[i] = m_datas_map_a[i].cData;
		res++;
	}
	return res;
}

int8_t I2C_ForEach::WriteVolume(uint32_t nCh, uint8_t cVolume) {
	uint32_t i = 0;
	for (i = 0; i < m_sizes_map; i++) {
		if (m_datas_map_v[i].cVal == (uint8_t)nCh) break;
	}
	if (i == m_sizes_map) return -1;
	m_datas_map_v[i].cData = cVolume;
	if (m_datas_map_v[i].wAddr > 0) {		
		if (!Write8bit(&m_datas_map_v[i])) {
			_DarkLogW("I2C Write failed, slave 0x%02X, vol addr=0x%02X\n", m_slaveAddr, m_datas_map_v[i].wAddr);
			return -2;
		}
	}
	return 0;
}

int32_t I2C_ForEach::GetWritedVolumes(uint32_t nBufSize, int32_t* OUT_pCh, int32_t* OUT_pVol)
{
	int32_t res = 0;
	uint32_t i;
	for (i = 0; i < m_sizes_map && i < nBufSize; i++) {
		OUT_pCh[i] = m_datas_map_v[i].cVal;
		OUT_pVol[i] = m_datas_map_v[i].cData;
		res++;
	}
	return res;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I2C_Interface

I2C_Interface::I2C_Interface()
{
	m_pGPIO = NULL;
	m_szDevName = NULL;	
	m_switchAddr = 0;
	memset(m_channels, 0, sizeof(int32_t) * I2C_MAX_CHANNEL);
	memset(m_values, 0, sizeof(int32_t) * I2C_MAX_CHANNEL);
}

I2C_Interface::~I2C_Interface()
{
	if (m_szDevName != NULL) delete[] m_szDevName;
}


int32_t I2C_Interface::Init(const char *szConfPath, GPIO_Control *pGPIO)
{
	Information_Parser IFP;
	pTAGInformation pTag, pSub, pSub2, pSub3, pSub4;
	m_pGPIO = pGPIO;

    char *szTemp = new char[DARK_MAXPATH];
    snprintf(szTemp, DARK_MAXPATH, "%s/i2c_conf.ifp", szConfPath);
    if (!IFP.OpenInfoFile(szTemp)) {
        _DarkLogE("WARN: Init - Opening config file..[%s]\n", szTemp);
		delete[] szTemp;
        return 0;
    }
	delete[] szTemp;

	if ((pTag = IFP.GetTag("hitek_i2c_config")) == NULL) {
		_DarkLogW("No I2C Configuration\n");
		return 0;
	}

	if ((pSub = IFP.GetTag("dev_name", pTag)) == NULL) {
		_DarkLogW("Invalid I2C Configuration\n");
		return -3;
	}

	size_t len = strlen((const char*)pSub->pBuffer);
	m_szDevName = new char[len + 1];
	DARK_SNPRINTF2(m_szDevName, len + 1, "%s", (char*)pSub->pBuffer);
	_DarkLogD("I2C Configuration found, Target device name = %s\n", m_szDevName);

	int32_t iAddr, iAddr2, iAddr3, iT;

	if ((pSub = IFP.GetTag("i2c_switch_addr", pTag)) != NULL) {
		m_switchAddr = (uint8_t) GCMN_Hex2Int((const char*)pSub->pBuffer);
		_DarkLogD("I2C Switch configured = %X\n", m_switchAddr);
	}

	if ((pSub = IFP.GetTag("reset", pTag)) != NULL) {
		int iResetPins[] = { 0, 0 };

		iAddr2 = 0;
		iAddr3 = 0;
		iT = 0;
		if ((pSub2 = IFP.GetTag("gpio_pin", pSub)) != NULL) iResetPins[0] = GCMN_Hex2Int((const char*)pSub2->pBuffer);
		if ((pSub2 = IFP.GetTag("gpio_pin2", pSub)) != NULL) iResetPins[1] = GCMN_Hex2Int((const char*)pSub2->pBuffer);
		if ((pSub2 = IFP.GetTag("active_low", pSub)) != NULL) {
			if (strcmp((const char*)pSub2->pBuffer, "true") == 0) iT = 0;
			else iT = 1;
		}
		if ((pSub2 = IFP.GetTag("delay_active", pSub)) != NULL) iAddr2 = GCMN_Hex2Int((const char*)pSub2->pBuffer);
		if ((pSub2 = IFP.GetTag("delay_after", pSub)) != NULL) iAddr3 = GCMN_Hex2Int((const char*)pSub2->pBuffer);
		if (iResetPins[0] > 0 || iResetPins[1] > 0) {
			_DarkLogD("I2C reset logic, pin=%d,%d, active=%d, active delay=%dms, after delay=%dms", iResetPins[0], iResetPins[1], iT, iAddr2, iAddr3);
			char szT[64];
			if (iResetPins[0] > 0 && m_pGPIO != NULL) {
				m_pGPIO->Write((uint32_t)iResetPins[0], (uint8_t)iT);
			}
			if (iResetPins[1] > 0 && m_pGPIO != NULL) {
				m_pGPIO->Write((uint32_t)iResetPins[1], (uint8_t)iT);
			}
			DARK_SLEEP(iAddr2);
			if (iResetPins[0] > 0 && m_pGPIO != NULL) {
				m_pGPIO->Write((uint32_t)iResetPins[0], iT == 0 ? 1 : 0);
			}
			if (iResetPins[1] > 0 && m_pGPIO != NULL) {
				DARK_SLEEP(iAddr2);
				m_pGPIO->Write((uint32_t)iResetPins[1], iT == 0 ? 1 : 0);
			}
			DARK_SLEEP(iAddr3);
		}
	}

	pSub = IFP.GetTag("device", pTag);
	uint32_t nCnt = 0;
	while (pSub != NULL) {
		nCnt++;
		pSub = pSub->pNext;
	}
	if (nCnt < 1) {
		_DarkLogE("Invalid I2C configuration, no sub device\n");
		return -4;
	}
	if (nCnt > I2C_MAX_INTERFACE) {
		_DarkLogE("Too many I2C configuration\n");
		return -5;
	}

	I2C_ForEach *pU, *pU2;
	bool bRes;

	pSub = IFP.GetTag("device", pTag);
	while (pSub != NULL) {
		iAddr = 0;
		if ((pSub2 = IFP.GetTag("slave_addr", pSub)) != NULL) iAddr = GCMN_Hex2Int((const char*)pSub2->pBuffer);

		pU = m_list.Add();
		pU->m_slaveAddr = (uint8_t)iAddr;

		if ((pSub2 = IFP.GetTag("switch_line", pSub)) != NULL) {
			iAddr = GCMN_Hex2Int((const char*)pSub2->pBuffer);
			pU->m_switchLine = (uint8_t)iAddr;
			pU->m_bUseSwitch = true;
		}
		else { //중복 주소 체크
			bRes = false;
			m_list.Rewind();
			while ((pU2 = m_list.GetNext()) != NULL) {
				if (pU == pU2) continue;
				if (pU->m_slaveAddr == pU2->m_slaveAddr) {
					bRes = true;
					break;
				}
			}
			if (bRes) {
				_DarkLogE("Duplicated I2C information exist\n");
				return -6;
			}
		}

		if ((pSub2 = IFP.GetTag("check", pSub)) != NULL) {
			pSub3 = IFP.GetTag("addr", pSub2);
			pSub4 = IFP.GetTag("data", pSub2);
			if (pSub3 == NULL || pSub4 == NULL) {
				_DarkLogE("Invalid I2C Config - check\n");
				return -7;
			}
			if (pU->SetupData_Check((char*)pSub3->pBuffer, (char*)pSub4->pBuffer) != true) {
				_DarkLogE("Invalid I2C Config - check - setup\n");
				return -8;
			}
		}
		if ((pSub2 = IFP.GetTag("init", pSub)) != NULL) {
			pSub3 = IFP.GetTag("addr", pSub2);
			pSub4 = IFP.GetTag("data", pSub2);
			if (pSub3 == NULL || pSub4 == NULL) {
				_DarkLogE("Invalid I2C Config - init\n");
				return -9;
			}
			if (pU->SetupData_Init((char*)pSub3->pBuffer, (char*)pSub4->pBuffer) != true) {
				_DarkLogE("Invalid I2C Config - init - setup\n");
				return -10;
			}
		}
		nCnt = 0;
		pSub2 = IFP.GetTag("ch_map", pSub);
		while (pSub2 != NULL) {
			pSub2 = pSub2->pNext;
			nCnt++;
		}

		pU->ChMap_Init(nCnt);

		pSub2 = IFP.GetTag("ch_map", pSub);
		while (pSub2 != NULL) {
			pSub3 = IFP.GetTag("channel", pSub2);
			if (pSub3 == NULL) {
				_DarkLogE("Invalid I2C Config - ch_map\n");
				return -11;
			}
			iAddr = (int32_t)GCMN_Hex2Int((const char*)pSub3->pBuffer);
			iAddr2 = 0;
			iAddr3 = 0;
			if ((pSub3 = IFP.GetTag("vol_reg", pSub2)) != NULL) {
				iAddr2 = (int32_t)GCMN_Hex2Int((const char*)pSub3->pBuffer);
			}
			if ((pSub3 = IFP.GetTag("attr_reg", pSub2)) != NULL) {
				iAddr3 = (int32_t)GCMN_Hex2Int((const char*)pSub3->pBuffer);
			}
			pU->ChMap_Setup((uint32_t)iAddr, (uint8_t)iAddr2, (uint8_t)iAddr3);

			pSub2 = pSub2->pNext;
		}


		pSub = pSub->pNext;
	}

	if (m_switchAddr > 0) {
#ifndef _MSC_VER
		if ((m_fp = open(m_szDevName, O_RDWR)) < 0) {
			_DarkLogE("File open failed for I2C Switch: %s", m_szDevName);
			return -12;
		}
#endif
		_DarkLogD("I2C Switch device 0x%X open complete", m_switchAddr);
	}

	m_list.Rewind();
	while ((pU = m_list.GetNext()) != NULL) {
		if (pU->Open(this, m_szDevName) != 0) {
			_DarkLogE("Device open failed\n");
			return -12;
		}
		if (pU->CheckAndInit() != 0) {
			_DarkLogE("Device init failed\n");
			return -13;
		}
	}

	ReadLastVolume();
	return 0;
}

void I2C_Interface::DoSystemLoop(uint32_t uiTick)
{
}

int32_t I2C_Interface::GetChAttributes(Json::Value& OUT_res)
{
	I2C_ForEach* pU;

	int32_t iRes = 0;
	int32_t iRet, i;
	Json::Value chi;

	m_list.Rewind();
	while ((pU = m_list.GetNext()) != NULL) {
		iRet = pU->ReadAttribute(I2C_MAX_CHANNEL, m_channels, m_values);
		if (iRet > 0) {			
			for (i = 0; i < iRet; i++) {
				chi.clear();
				chi["channel"] = m_channels[i];
				chi["attribute"] = m_values[i];				
				OUT_res["attributes"].append(chi);
				iRes++;
			}
		}
	}

	return iRes;
}

int32_t I2C_Interface::GetChVolumes(Json::Value& OUT_res)
{
	I2C_ForEach* pU;

	int32_t iRes = 0;
	int32_t iRet, i;
	Json::Value chi;

	m_list.Rewind();
	while ((pU = m_list.GetNext()) != NULL) {
		iRet = pU->ReadVolumes(I2C_MAX_CHANNEL, m_channels, m_values);
		if (iRet > 0) {
			for (i = 0; i < iRet; i++) {
				chi.clear();
				chi["channel"] = m_channels[i];
				chi["volume"] = m_values[i];
				OUT_res["volumes"].append(chi);
				iRes++;
			}
		}
	}

	return iRes;
}

int32_t I2C_Interface::SetChVolumes(Json::Value& volArr, Json::Value& OUT_res)
{
	I2C_ForEach* pU;
	int32_t i, c, v, ret;
	int32_t res = 0;
	Json::Value chi;

	for (i = 0; i < volArr.size(); i++) {
		c = volArr[i]["channel"].asInt();
		v = volArr[i]["volume"].asInt();
		ret = 0;
		m_list.Rewind();
		while ((pU = m_list.GetNext()) != NULL) {
			ret = pU->WriteVolume((uint32_t)c, (uint8_t)v);
			if (ret == 0) res++;
			if (ret != -1) break;
		}
		chi.clear();
		chi["channel"] = c;
		if (ret != 0) {
			_DarkLogW("Warn: SetChVolumes - No valid i2c device for channel %d", c);			
			chi["res"] = false;
		}
		else chi["res"] = true;
		OUT_res["results"].append(chi);
	}
	WriteLastVolume();
	return res;
}

void I2C_Interface::DoLineSwitch(I2C_ForEach* pIFE) {
	if (m_preI2C == pIFE) return;

	unsigned char uT[1];
	uT[0] = (1 << pIFE->m_switchLine);

#ifndef _MSC_VER
	ioctl(m_fp, I2C_SLAVE, m_switchAddr);
	write(m_fp, uT, 1);
	ioctl(m_fp, I2C_SLAVE, pIFE->m_slaveAddr);
#endif
	
	m_preI2C = pIFE;
}

bool I2C_Interface::Read8bit(I2C_ForEach* pIFE, i2c_8bit_pair* pPair) {
	if (m_fp < 0) return false;
	DoLineSwitch(pIFE);
#ifndef _MSC_VER
	uint8_t addr = (uint8_t)pPair->wAddr;
	uint8_t data = 0;

	struct i2c_msg msg[2];
	struct i2c_rdwr_ioctl_data idata;
	msg[0].addr = (__u16)pIFE->m_slaveAddr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;
	msg[1].addr = (__u16)pIFE->m_slaveAddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &data;

	idata.msgs = msg;
	idata.nmsgs = 2;

	if (ioctl(m_fp, I2C_RDWR, &idata) < 0) {
		return false;
	}

	pPair->cData = data;
#endif
	return true;
}

bool I2C_Interface::Write8bit(I2C_ForEach* pIFE, i2c_8bit_pair* pPair) {
	if (m_fp < 0) return false;
	DoLineSwitch(pIFE);
#ifndef _MSC_VER
	uint8_t buf[2];
	buf[0] = (uint8_t)pPair->wAddr;
	buf[1] = pPair->cData;
	if (write(m_fp, buf, 2) != 2) {
		return false;
	}
#endif
	return true;
}

void I2C_Interface::WriteLastVolume() {
	I2C_ForEach* pU;
	int32_t iRet, i;
	
	FILE* fp;
	DARKPIF_fopen(&fp, I2C_LAST_VOLUME_FILE, "w+");
	fprintf(fp, "<i2c_volumes>\n");

	m_list.Rewind();
	while ((pU = m_list.GetNext()) != NULL) {
		iRet = pU->GetWritedVolumes(I2C_MAX_CHANNEL, m_channels, m_values);
		if (iRet > 0) {
			for (i = 0; i < iRet; i++) {
				fprintf(fp, "\t<volume>\n");
				fprintf(fp, "\t\t<ch>%d</ch>\n", m_channels[i]);
				fprintf(fp, "\t\t<val>%d</val>\n", m_values[i]);
				fprintf(fp, "\t</volume>\n");
			}
		}
	}
	fprintf(fp, "</i2c_volumes>\n");
	fclose(fp);
}

void I2C_Interface::ReadLastVolume() {
	Information_Parser IFP;
	pTAGInformation pRoot, pTag, pSub;
	int32_t c, v, ret;
	I2C_ForEach* pU;

	if (!IFP.OpenInfoFile(I2C_LAST_VOLUME_FILE)) {
		return;
	}

	pRoot = IFP.GetTag("i2c_volumes");
	if (pRoot == NULL) {
		_DarkLogE("ERROR: Invalid I2C Volume file format");		
	}

	pTag = IFP.GetTag("volume", pRoot);
	while (pTag != NULL) {
		c = -1;		
		if ((pSub = IFP.GetTag("ch", pTag)) != NULL) c = atoi((char*)pSub->pBuffer);
		if ((pSub = IFP.GetTag("val", pTag)) != NULL) v = atoi((char*)pSub->pBuffer);

		if (c >= 0) {
			_DarkLogD("Write last volume: ch=%d, val=%d", c, v);
			m_list.Rewind();
			while ((pU = m_list.GetNext()) != NULL) {
				ret = pU->WriteVolume((uint32_t)c, (uint8_t)v);
				if (ret != -1) break;
			}
		}

		pTag = pTag->pNext;
	}
}
