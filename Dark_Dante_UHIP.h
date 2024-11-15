#pragma once
#include "Dark_Transport.h"

class Dark_Dante_DevInfo {
public:
	Dark_Dante_DevInfo();
	~Dark_Dante_DevInfo();

	bool IsUpdated();
	void PrintDevInfo();

	unsigned int handle_ddp_device_general(void* pRInfo, uint16_t offset);
	unsigned int handle_ddp_device_id(void* pRInfo, uint16_t offset);

	char* m_szModel_id; //UltimoX4
	char* m_szDevId;
	char* m_szDevName;

	uint64_t m_n64_model_id; 
	uint32_t m_sw_ver; //S/W���� - 8_8_16
	uint32_t m_sw_build; //�������
	uint32_t m_fw_ver; //F/W���� - 8_8_16
	uint32_t m_fw_build; //�������
	uint32_t m_bootloader_ver;
	uint32_t m_bootloader_build;
	uint32_t m_api_ver;
	uint32_t m_cap_flags;
	uint32_t m_status_flags;

protected:
	uint32_t m_nUpdated;
};

class Dark_Dante_UHIP
{
public:
	Dark_Dante_UHIP();
	~Dark_Dante_UHIP();

	int32_t SetTransport(Dark_Transport* pTransport);
	int32_t requestGeneralInfo();
	int32_t requestDeviceId();

	int32_t GetDeviceInfo(bool bDoUpdate, Dark_Dante_DevInfo** ppInfo);

protected:
	Dark_Transport *m_pTransport;
	int32_t m_iState;
	uint8_t* m_cobs_rx_buffer;
	uint8_t* m_cobs_tx_buffer;
	Dark_Dante_DevInfo *m_pDDDI;

	bool m_bIsInProc;
	uint32_t m_nProcStep;
	uint32_t m_nTimestamp;

#ifdef _MSC_VER
	static void _THR_Reader(void* pParam);
#else
	pthread_attr_t m_THRATT_detach;
	static void* _THR_Reader(void* pParam);
#endif

	int32_t _StopReader();
	void _PROC_Reader();

	void _processPacket(uint8_t* pBuf, int32_t iSize);
	void _process_cobs_rx_packet(size_t rx_packet_size_bytes);
	void _process_ddp_packet(uint8_t const* pbuf, uint16_t packet_length_bytes);
	void _sendACK();
	unsigned int _prepare_uhip_packet_and_send(uint8_t* tx_buf, uint16_t ddp_packet_len);
	int32_t _reqDevInfo(bool bGeneral);
};

