#pragma once
#include "common_inc.h"
#include "RyuLinkedList.h"
#include "GPIO_Control.h"

#define I2C_MAX_INTERFACE		16
#define I2C_MAX_CHANNEL			128

typedef struct i2c_8bit_pair {
	uint16_t wAddr;
	uint8_t cData;
	uint8_t cVal;
} i2c_8bit_pair;

class I2C_Interface;

class I2C_ForEach {
public:
	I2C_ForEach();
	~I2C_ForEach();

	int8_t Open(I2C_Interface *pParent, const char* szDevName);
	int8_t CheckAndInit();

	bool SetupData_Check(char* szAddr, char* szData);
	bool SetupData_Init(char* szAddr, char* szData);
	bool ChMap_Init(uint32_t nCount);
	bool ChMap_Setup(uint32_t nCh, uint8_t cVolAddr, uint8_t cAttrAddr);

	int32_t ReadVolumes(uint32_t nBufSize, int32_t *OUT_pCh, int32_t *OUT_pVol);
	int32_t ReadAttribute(uint32_t nBufSize, int32_t* OUT_pCh, int32_t* OUT_pAttr);
	int8_t WriteVolume(uint32_t nCh, uint8_t cVolume);

	uint8_t m_slaveAddr;
	uint8_t m_switchLine; //I2C 스위치 접속 정보
	bool m_bUseSwitch; //스위치 사용 여부

protected:
	I2C_Interface* m_pParent;
	int8_t m_cState;
	int32_t m_fp;	
	uint32_t m_sizes_check;
	uint32_t m_sizes_init;
	uint32_t m_sizes_map;
	i2c_8bit_pair* m_datas_check;
	i2c_8bit_pair* m_datas_init;
	i2c_8bit_pair* m_datas_map_v;
	i2c_8bit_pair* m_datas_map_a;
	
	bool Read8bit(i2c_8bit_pair* pPair);
	bool Write8bit(i2c_8bit_pair* pPair);
};

class I2C_Interface
{
public:
	I2C_Interface();
	~I2C_Interface();

	int32_t Init(const char *szConfPath, GPIO_Control *pGPIO);
	void DoSystemLoop(uint32_t uiTick);

	int32_t GetChAttributes(Json::Value& OUT_res);
	int32_t GetChVolumes(Json::Value& OUT_res);
	int32_t SetChVolumes(Json::Value& volArr, Json::Value& OUT_res);

protected:
	friend class I2C_ForEach;
	GPIO_Control *m_pGPIO;

	char* m_szDevName;
	int32_t m_fp;
	uint8_t m_switchAddr; //I2C Switch Address
	RyuLinkedList<I2C_ForEach> m_list;
	I2C_ForEach* m_preI2C; //전에 제어중이던 장비

	int32_t m_channels[I2C_MAX_CHANNEL];
	int32_t m_values[I2C_MAX_CHANNEL];

	void DoLineSwitch(I2C_ForEach* pIFE);
	bool Read8bit(I2C_ForEach* pIFE, i2c_8bit_pair* pPair);
	bool Write8bit(I2C_ForEach* pIFE, i2c_8bit_pair* pPair);
};

