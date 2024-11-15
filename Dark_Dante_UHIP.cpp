#include "Dark_Dante_UHIP.h"
extern "C" {
#	include "dante_dark_inte/include/cobs.h"
#	include "dante_dark_inte/include/uhip_packet_write.h"
#	include "dante_dark_inte/include/uhip_packet_read.h"
#	include "dante_dark_inte/include/frame.h"
#	include "dante_dark_inte/include/ddp/device_client.h"
#	include "dante_dark_inte/include/ddp/util.h"
}

#define _DarkLogD(fmt, args...)	printf("[UHIP_DEBUG]: " fmt, ##args)
#define _DarkLogE(fmt, args...)	printf("[UHIP_ERROR]: " fmt, ##args)
#define _DarkLogW(fmt, args...)	printf("[UHIP_WARNING]: " fmt, ##args)

Dark_Dante_DevInfo::Dark_Dante_DevInfo() {
	m_szModel_id = NULL;
	m_szDevId = NULL;
	m_szDevName = NULL;

	m_n64_model_id = 0;
	m_sw_ver = 0;
	m_sw_build = 0;
	m_fw_ver = 0;
	m_fw_build = 0;
	m_bootloader_ver = 0;
	m_bootloader_build = 0;
	m_api_ver = 0;
	m_cap_flags = 0;
	m_status_flags = 0;

	m_nUpdated = 0;
}

Dark_Dante_DevInfo::~Dark_Dante_DevInfo() {
	if (m_szModel_id != NULL) delete[] m_szModel_id;
	if (m_szDevId != NULL) delete[] m_szDevId;
	if (m_szDevName != NULL) delete[] m_szDevName;
}

bool Dark_Dante_DevInfo::IsUpdated() {
	if (m_nUpdated == 0x03) return true;
	return false;
}

unsigned int Dark_Dante_DevInfo::handle_ddp_device_general(void* pRInfo, uint16_t offset) {
	ddp_packet_read_info_t* ddp_rinfo = (ddp_packet_read_info_t*)pRInfo;

	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	char* model_id_str;
	dante_id64_t* model_id;	

	result = ddp_read_device_general_response(ddp_rinfo, offset, &request_id, &status,
		&model_id, &model_id_str, &m_sw_ver, &m_sw_build, &m_fw_ver, &m_fw_build,
		&m_bootloader_ver, &m_bootloader_build, &m_api_ver, &m_cap_flags, &m_status_flags);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	memcpy(&m_n64_model_id, model_id, sizeof(uint64_t));
	
	if (model_id_str) {
		if (m_szModel_id != NULL) delete[] m_szModel_id;
		m_szModel_id = new char[strlen(model_id_str) + 1];
		strcpy(m_szModel_id, model_id_str);
	}

	m_nUpdated |= 0x01;

	return AUD_SUCCESS;
}

unsigned int Dark_Dante_DevInfo::handle_ddp_device_id(void* pRInfo, uint16_t offset) {
	ddp_packet_read_info_t* ddp_rinfo = (ddp_packet_read_info_t*)pRInfo;

	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t status_flags, process_id;
	dante_device_id_t* device_id;
	dante_device_id_str_t device_id_string_buff;
	char const* default_name, * friendly_name, * domain, * advertised_name;

	result = ddp_read_device_identity_response(ddp_rinfo, offset, &request_id, &status, &status_flags, &process_id,
		&device_id, &default_name, &friendly_name, &domain, &advertised_name);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	//_DarkLogD("Identity status flags: 0x%04X\n", status_flags);
	//_DarkLogD("Process ID: %d\n", process_id);
	const char* szDevName = NULL;
	char* szDevId = dante_device_id_to_string(device_id, device_id_string_buff);
	
	if (m_szDevId != NULL) delete[] m_szDevId;
	m_szDevId = new char[strlen(szDevId) + 1];
	strcpy(m_szDevId, szDevId);

	if (default_name) szDevName = default_name;	
	else if (friendly_name) szDevName = friendly_name;	

	if (szDevName != NULL) {
		if (m_szDevName != NULL) delete[] m_szDevName;
		m_szDevName = new char[strlen(szDevName) + 1];
		strcpy(m_szDevName, szDevName);
	}
	//if (domain) {
	//	_DarkLogD("Dante domain: %s\n", domain);
	//}
	//if (advertised_name) {
	//	_DarkLogD("Advertised name: %s\n", advertised_name);
	//}
	m_nUpdated |= 0x02;

	return AUD_SUCCESS;
}

void Dark_Dante_DevInfo::PrintDevInfo() {
	if (!IsUpdated()) {
		printf("ERROR - Not updated!!!\n");
		return;
	}

	dante_id64_t* model_id;
	char id64_t_string_buff[DANTE_ID64_DNSSD_BUF_LENGTH];

	model_id = (dante_id64_t*)&m_n64_model_id;
	printf("Dark Dante Device Info\n");
	printf("\tDevice ID: %s\n", m_szDevId);
	printf("\tDevice Name: ");
	if (m_szDevName != NULL) printf("%s\n", m_szDevName);
	else printf("Not Set!!\n");
	printf("\tModel ID: 0x%s\n", dante_id64_to_dnssd_hex(model_id, &id64_t_string_buff[0]));
	printf("\tModel ID string: ");
	if (m_szModel_id != NULL) printf("%s\n", m_szModel_id);
	else printf("Not set!\n");

	printf("\tVersion&Flag Info\n");
	printf("\t\tSoftware version: %d.%d.%d\n", m_sw_ver >> 24, (m_sw_ver >> 16) & 0xFF, m_sw_ver & 0xFFFF);
	printf("\t\tSoftware build: %d\n", m_sw_build);
	printf("\t\tFirmware version: %d.%d.%d\n", m_fw_ver >> 24, (m_fw_ver >> 16) & 0xFF, m_fw_ver & 0xFFFF);
	printf("\t\tFirmware build: %d\n", m_fw_build);
	printf("\t\tBootloader version: %d\n", m_bootloader_ver);
	printf("\t\tBootloader build: %d\n", m_bootloader_build);
	printf("\t\tAPI version: %d\n", m_api_ver);
	printf("\t\tCapability flags: 0x%08X\n", m_cap_flags);
	printf("\t\tStatus flags: 0x%08X\n", m_status_flags);
}


/// ///////////////////////////////////////////////////////////////////////////////
Dark_Dante_UHIP::Dark_Dante_UHIP() {
	m_pTransport = NULL;

	m_iState = 0;

	m_cobs_rx_buffer = new uint8_t[_COBS_BUFFER_PAD(UHIP_PACKET_SIZE_MAX) + UHIP_PACKET_SIZE_MAX + COBS_ZERO_PAIR_NUM];
	m_cobs_tx_buffer = new uint8_t[_DMA_CHUNK_ROUND(_COBS_BUFFER_PAD(UHIP_PACKET_SIZE_MAX) + UHIP_PACKET_SIZE_MAX + COBS_ZERO_PAIR_NUM)];

#ifndef _MSC_VER
	pthread_attr_init(&m_THRATT_detach);
	pthread_attr_setdetachstate(&m_THRATT_detach, PTHREAD_CREATE_DETACHED);
#endif

	m_bIsInProc = false;
	m_nProcStep = 0;
	m_nTimestamp = 0;
	m_pDDDI = new Dark_Dante_DevInfo();
}

Dark_Dante_UHIP::~Dark_Dante_UHIP() {
	_StopReader();
	delete[] m_cobs_rx_buffer;
	delete[] m_cobs_tx_buffer;
	delete m_pDDDI;
}

int32_t Dark_Dante_UHIP::SetTransport(Dark_Transport* pTransport) {
	if (m_iState > 0) {
		_DarkLogE("ERROR - SetTransport - Started already!!!\n");
		return -1;
	}
	m_pTransport = pTransport;

	m_iState = 1;
#ifdef _MSC_VER	
	_beginthread(_THR_Reader, 0, this);
#else
	pthread_t pThr;
	if (pthread_create(&pThr, &m_THRATT_detach, _THR_Reader, this) != 0) {
		_DarkLogE("Reader thread creation failed!!!\n");
		m_iState = 0;
		return -2;
	}
#endif
	return 0;
}

int32_t Dark_Dante_UHIP::_StopReader() {
	if (m_iState != 1) return -1;
	m_iState |= 0x02;
	int32_t iCnt = 0;
	while (!(m_iState & 0x04)) {
		iCnt++;
		if (iCnt > 10) {
			_DarkLogE("ERROR - Stopping timeout\n");
			break;
		}
		DARK_SLEEP(100);
	}
	m_iState = 0;
	return 0;
}

#ifdef _MSC_VER
void Dark_Dante_UHIP::_THR_Reader(void* pParam)
#else
void* Dark_Dante_UHIP::_THR_Reader(void* pParam)
#endif
{
	Dark_Dante_UHIP* pP = (Dark_Dante_UHIP *)pParam;
	pP->_PROC_Reader();
#ifndef _MSC_VER
	return 0;
#endif
}

void Dark_Dante_UHIP::_PROC_Reader()
{
	int32_t iRecv, iRes, iDatasize;
	uint8_t *pBuf = NULL;
	pBuf = new uint8_t[DARK_MAXPATH];
	_DarkLogD("Reader Thread start\n");

	iRecv = 0;
	
	while ((m_iState & 0x02) != 0x02) {
		iRes = m_pTransport->dark_read(pBuf + iRecv, DARK_MAXPATH);
		if (iRes < 0) {
			_DarkLogW("Reader thread - read error - [%d]\n", iRes);
			break;
		}
		else if (iRes == 0) {
			DARK_SLEEP(10);
			continue;
		}

		iRecv += iRes;
		if (iRecv >= UHIP_CHUNK_SIZE) {
			iDatasize = iRecv - (iRecv % UHIP_CHUNK_SIZE);
			//printf("ProcessPkt: %d / %d\n", iDatasize, iRecv);
			_processPacket(pBuf, iDatasize);
			if (iDatasize < iRecv) memmove(pBuf, pBuf + iDatasize, iRecv - iDatasize);
			iRecv -= iDatasize;
			//printf("Remain = %d\n", iRecv);
		}
	}

	_DarkLogD("Reader thread finished\n");
	delete[] pBuf;
	m_iState |= 0x04;
}

void Dark_Dante_UHIP::_processPacket(uint8_t* pBuf, int32_t iSize) {
	aud_error_t result;
	size_t extracted_size;	
	int32_t i;

	for (i = 0; i < iSize; ++i)
	{
		//?갡 ???ο??? ???? ??????? ??????? ?? ???..
		result = extract_cobs_from_rx_frame(pBuf[i], m_cobs_rx_buffer, &extracted_size, NULL, NULL, 1000);

		if (result == AUD_ERR_NOMEMORY)
		{
			_DarkLogE("_processPacket: - AUD_ERR_NOMEMORY\n");
			return;
		}
		else if (result == AUD_SUCCESS)
		{
			//printf("extract_cobs_from_rx_frame success: %d / %d\n", i, iSize);
			_process_cobs_rx_packet(extracted_size);			
		}
		else if (result != AUD_ERR_INPROGRESS) {
			_DarkLogE("WTF ERROR: %d = %d\n", i, result);
		}
	}

	/*
	if (iProcessed > 0 && (iProcessed % UHIP_CHUNK_SIZE) > 0) {
		iProcessed += UHIP_CHUNK_SIZE - (iProcessed % UHIP_CHUNK_SIZE); //????? Chunk?????? ????? FF?? ????? ?????? ????
		if (iProcessed > iSize) {
			_DarkLogE("WTF!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d / %d\n", iProcessed, iSize);
			return iSize;
		}
	}

	return iProcessed;
	*/
}

void Dark_Dante_UHIP::_process_cobs_rx_packet(size_t rx_packet_size_bytes)
{
	aud_error_t result;
	size_t uncobs_size = 0;
	uhip_packet_read_info_t uhip_read_info;
	UhipPacketType uhip_packet_type;

	uint8_t* rx_buffer = m_cobs_rx_buffer;

	//uncobs the packet
	uncobs_size = cobs_decode(m_cobs_rx_buffer, rx_packet_size_bytes, rx_buffer, UHIP_PACKET_SIZE_MAX);
	//printf("process_cobs_rx_packet: %d\n", uncobs_size);

	//check for a corrupted packet
	if (uncobs_size == 0)
	{
		_DarkLogE("ERROR COBS decode error - discarding packet!\n");
		return;
	}

	//read the UHIP header
	result = uhip_packet_init_read(&uhip_read_info, (uint32_t*)rx_buffer, (uint16_t)uncobs_size);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR UHIP malformed [uhip_packet_init_read()] - discarding packet!\n");
		return;
	}

	//parse based on packet type
	result = uhip_packet_read_get_type(&uhip_read_info, &uhip_packet_type);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR UHIP malformed [uhip_packet_read_get_type()] - discarding packet!\n");
		return;
	}

	switch (uhip_packet_type)
	{
	case UhipPacketType_PROTOCOL_CONTROL:
	{
		UhipProtocolControlType uhip_prot_control_type;
		if (uhip_packet_read_protocol_control(&uhip_read_info, &uhip_prot_control_type) == AUD_SUCCESS)
		{
			switch (uhip_prot_control_type)
			{
			case UhipProtocolControlType_ACK_SUCCESS:
				//AUD_PRINTF("OK: UhipPacketType_PROTOCOL_CONTROL - ACK\n");
				break;
			case UhipProtocolControlType_ERROR_OTHER:
			case UhipProtocolControlType_ERROR_MALFORMED_PACKET:
			case UhipProtocolControlType_ERROR_UNKNOWN_TYPE:
			case UhipProtocolControlType_ERROR_NETWORK:
			case UhipProtocolControlType_ERROR_TIMEOUT:
				_DarkLogE("ERR: UhipPacketType_PROTOCOL_CONTROL - ERR: %d\n", uhip_prot_control_type);
				break;
			}
		}
	}
	break;

	case UhipPacketType_CONMON_PACKET_BRIDGE:
	{
		_sendACK();
		UhipConMonPacketBridgeChannel cmc_pb_chan;
		uint8_t* payload;
		uint16_t payload_length;

		//parse the packet
		if (uhip_packet_read_cmc_packet_bridge(&uhip_read_info, &cmc_pb_chan, &payload, &payload_length) == AUD_SUCCESS)
		{
			//print header
			//_DarkLogD("Received ConMon Packet Bridge Packet, payload_length=%u\n", payload_length);
		}
		else _DarkLogE("FAIL: uhip_packet_read_cmc_packet_bridge\n");
	}
	break;

	case UhipPacketType_UDP_PACKET_BRIDGE:
		_sendACK();
		_DarkLogE("_process_cobs_rx_packet: ERROR - Impossible recv UDP Packet bridge!\n");
		break;
	case UhipPacketType_DANTE_EVENTS:
	{
		_sendACK();
		ddp_packet_read_info_t ddp_rinfo;
		ddp_message_read_info_t ddp_msg_info;
		uint16_t packet_offset = 0;

		result = ddp_packet_init_read(&ddp_rinfo, (uint32_t*)(rx_buffer + sizeof(uhip_header_t) + sizeof(ddp_packet_header_t)), (ddp_size_t)(uncobs_size - sizeof(uhip_header_t)));
		if (result != AUD_SUCCESS)
		{
			_DarkLogE("WTF!!!!!!!!!! - ddp_packet_init_read\n");
			return;
		}
		result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);

		//read every TLV block in the packet
		while (result == AUD_SUCCESS)
		{
			_DarkLogD("Received Dante Event %s\n", ddp_opcode_to_string(ddp_msg_info.opcode));
			//jump to the next TLV block
			packet_offset += ddp_msg_info.length_bytes;
			result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);
		} //while (result == AUD_SUCCESS)
	}
	break;

	case UhipPacketType_DDP:
		_sendACK();
		_process_ddp_packet(rx_buffer + sizeof(uhip_header_t), (ddp_size_t)(uncobs_size - sizeof(uhip_header_t)));
		break;
	default:
		_DarkLogE("What the fucking unknown type: %d\n", uhip_packet_type);
		break;
	}
}

void Dark_Dante_UHIP::_process_ddp_packet(uint8_t const* pbuf, uint16_t packet_length_bytes)
{
	aud_error_t result;
	ddp_packet_read_info_t ddp_rinfo;
	ddp_message_read_info_t ddp_msg_info;
	ddp_request_id_t response_request_id;
	ddp_status_t response_status;
	uint16_t packet_offset = 0;
	ddp_block_header_t block_header;

	//parse the DDP message
	result = ddp_packet_init_read(&ddp_rinfo, (uint32_t*)pbuf, packet_length_bytes);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("Failed to initialize DDP packet read\n");
		return;
	}

	result = ddp_packet_read_block_header(&ddp_rinfo, packet_offset, &block_header);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("Failed to read block header\n");
		return;
	}
	if (block_header._.opcode != DDP_OP_PACKET_HEADER)
	{
		_DarkLogE("Unexpected opcode: 0x%04X in the block header, expected 0x0001\n", block_header._.opcode);
		return;
	}

	// Jump past the block header to the start of the message in this DDP packet
	packet_offset += block_header._.length_bytes;
	result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);

	if (result != AUD_SUCCESS)
	{
		_DarkLogE("Unable to read DDP message\n");
		return;
	}

	//read every TLV block in the packet
	while (result == AUD_SUCCESS)
	{
		//check the status for any errors
		response_status = ddp_packet_read_response_status(&ddp_msg_info);
		if (response_status != DDP_STATUS_NOERROR)
		{
			if ((ddp_msg_info.opcode == DDP_OP_DEVICE_UPGRADE) ||
				(ddp_msg_info.opcode == DDP_OP_DEVICE_AES67))
			{
				_DarkLogD("DDP %s message received with status: %s\n", ddp_opcode_to_string(ddp_msg_info.opcode), ddp_status_to_string(response_status));
			}
			else
			{
				_DarkLogE("ERROR DDP message received with status %s for opcode %s - discarding packet\n", ddp_status_to_string(response_status), ddp_opcode_to_string(ddp_msg_info.opcode));
			}
			return;
		}

		//read the request id
		response_request_id = ddp_packet_read_response_request_id(&ddp_msg_info);
		_DarkLogD("Received %s for %s\n", (response_request_id) ? "response" : "event", ddp_opcode_to_string(ddp_msg_info.opcode));
		//TX REQ ID?? ???? ID?? ?? ??? ?????? ???....

		switch (ddp_msg_info.opcode)
		{
		case DDP_OP_DEVICE_GENERAL:
			result = m_pDDDI->handle_ddp_device_general(&ddp_rinfo, packet_offset);
			if (m_nProcStep == 1) m_bIsInProc = false;
			else if (m_nProcStep == 3) _reqDevInfo(false); //???			
			break;
		case DDP_OP_DEVICE_IDENTITY:
			result = m_pDDDI->handle_ddp_device_id(&ddp_rinfo, packet_offset);
			if (m_nProcStep == 2) m_bIsInProc = false;
			break;
		case DDP_OP_DEVICE_MANF:
		case DDP_OP_DEVICE_UPGRADE:
		case DDP_OP_DEVICE_ERASE_CONFIG:
		case DDP_OP_DEVICE_REBOOT:
		case DDP_OP_DEVICE_GPIO:
		case DDP_OP_DEVICE_SWITCH_LED:
		case DDP_OP_DEVICE_LOCK_UNLOCK:
		case DDP_OP_DEVICE_AES67:
		case DDP_OP_DEVICE_SWITCH_REDUNDANCY:
		case DDP_OP_DEVICE_UART_CONFIG:
		case DDP_OP_DEVICE_VLAN_CONFIG:
		case DDP_OP_DEVICE_METER_CONFIG:
		case DDP_OP_DEVICE_DANTE_DOMAIN:
		case DDP_OP_DEVICE_SWITCH_STATUS:
		case DDP_OP_NETWORK_BASIC: //??? ?????? ?? ??........
		case DDP_OP_NETWORK_CONFIG:
		case DDP_OP_CLOCK_BASIC_LEGACY:
		case DDP_OP_CLOCK_CONFIG:
		case DDP_OP_CLOCK_PULLUP:
		case DDP_OP_CLOCK_BASIC2:
		case DDP_OP_CLOCK_V2_QUERY:
		case DDP_OP_AUDIO_BASIC:
		case DDP_OP_AUDIO_SRATE_CONFIG:
		case DDP_OP_AUDIO_ENC_CONFIG:
		case DDP_OP_AUDIO_INTERFACE:
		case DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG:
		case DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA:
		case DDP_OP_AUDIO_RTP_QUERY:
		case DDP_OP_ROUTING_BASIC:
		case DDP_OP_ROUTING_READY_STATE:
		case DDP_OP_ROUTING_PERFORMANCE_CONFIG:
		case DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE:
		case DDP_OP_ROUTING_RX_SUBSCRIBE_SET:
		case DDP_OP_ROUTING_RX_CHAN_LABEL_SET:
		case DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE:
		case DDP_OP_ROUTING_TX_CHAN_LABEL_SET:
		case DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE:
		case DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET:
		case DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE:
		case DDP_OP_ROUTING_RX_CHAN_STATUS:
		case DDP_OP_ROUTING_RX_FLOW_STATUS:
		case DDP_OP_ROUTING_RX_UNSUB_CHAN:
		case DDP_OP_ROUTING_FLOW_DELETE:
		case DDP_OP_ROUTING_RTP_RX_FLOW_CONFIG_SET:
		case DDP_OP_ROUTING_RTP_TX_FLOW_CONFIG_SET:
		case DDP_OP_SWITCH_REG_ACCESS:
			_DarkLogD("Skip DDP msg: opcode=0x%04x - %s\n", ddp_msg_info.opcode, ddp_opcode_to_string(ddp_msg_info.opcode));
			break;
		default:
			_DarkLogE("Unsupported / Unknown DDP msg: opcode=0x%04x - %s\n", ddp_msg_info.opcode, ddp_opcode_to_string(ddp_msg_info.opcode));
			break;
		} //switch (ddp_msg_info.opcode)

		  //jump to the next TLV block
		packet_offset += ddp_msg_info.length_bytes;
		result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);
	} //while (result == AUD_SUCCESS)

	return;
}


void Dark_Dante_UHIP::_sendACK() {
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;

	// Allocate extra room to store the start of packet delimiter and data inserted after the COBS encode
	uint8_t* ctrl_buf_ptr = m_cobs_tx_buffer + 1 + _COBS_BUFFER_PAD(UHIP_PACKET_SIZE_MAX);

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t*)ctrl_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR: _sendACK - uhip_packet_init_write\n");
		return;
	}

	// 2. Create the UHIP packet with the DDP packet created from step 3
	result = uhip_packet_write_protocol_control(&uhip_winfo, UhipProtocolControlType_ACK_SUCCESS);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR: _sendACK - uhip_packet_write_protocol_control\n");
		return;
	}

	// 3. COBS encode the UHIP packet from step 2. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(ctrl_buf_ptr, uhip_winfo.curr_length_bytes, m_cobs_tx_buffer + 1);

	// 4. Add the delimeter bytes to frame and add pad bytes to make the data to be sent a multiple of 32-bytes
	total_data_to_send = prepare_tx_frame(m_cobs_tx_buffer, cobs_encoded_size);
	if (total_data_to_send == 0)
	{
		_DarkLogE("ERROR: _sendACK - prepare_tx_frame\n");
		return;
	}

	// 5. Send the data to the Dante device
	bytes_sent = m_pTransport->dark_write(m_cobs_tx_buffer, total_data_to_send);
	if (bytes_sent != total_data_to_send)
	{
		_DarkLogE("ERROR: _sendACK - m_pTransport->dark_write\n");
		return;
	}

	return;
}

unsigned int Dark_Dante_UHIP::_prepare_uhip_packet_and_send(uint8_t* tx_buf, uint16_t ddp_packet_len)
{
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	uint8_t* ddp_tx_buffer_ptr;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;

	// Allow room to hold the start of packet delimiter and data inserted after the COBS encode
	uint8_t* tx_buf_ptr = tx_buf + 1 + _COBS_BUFFER_PAD(UHIP_PACKET_SIZE_MAX);

	ddp_tx_buffer_ptr = &tx_buf_ptr[sizeof(uhip_header_t)];

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t*)tx_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 2. Create the UHIP packet with the DDP packet. Reusing same buffer. Pass no memory pointer and length as 0.
	result = uhip_packet_write_dante_device_protcol(&uhip_winfo, (uint32_t*)ddp_tx_buffer_ptr, ddp_packet_len);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. COBS encode the UHIP+DDP packet from step 6. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(tx_buf_ptr, uhip_winfo.curr_length_bytes, tx_buf + 1);

	// 4. Add the delimeter bytes to frame and add pad bytes to make the data to be sent a multiple of 32-bytes
	total_data_to_send = prepare_tx_frame(tx_buf, cobs_encoded_size);
	if (total_data_to_send == 0)
	{
		return AUD_ERR_SYSTEM;
	}

	// 5. Send the data to the Dante device
	bytes_sent = m_pTransport->dark_write(tx_buf, total_data_to_send);
	if (bytes_sent != total_data_to_send)
	{
		return AUD_ERR_SYSTEM;
	}

	return AUD_SUCCESS;
}

int32_t Dark_Dante_UHIP::_reqDevInfo(bool bGeneral) {
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	uint8_t* ddp_tx_buffer_ptr;
	ddp_size_t ddp_packet_len = 0;
	size_t ddp_buf_max_len, ddp_start_offset;

	ddp_request_id_t config_devg_request_id = 1;

	ddp_start_offset = sizeof(uhip_header_t) + 1 + _COBS_BUFFER_PAD(UHIP_PACKET_SIZE_MAX);
	ddp_tx_buffer_ptr = &m_cobs_tx_buffer[ddp_start_offset];
	ddp_buf_max_len = UHIP_PACKET_SIZE_MAX - ddp_start_offset;

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t*)ddp_tx_buffer_ptr, (ddp_size_t)ddp_buf_max_len);
	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR: _reqDevInfoWTF - ddp_packet_init_write\n");
		return -1;
	}

	// Create the DDP netwok basic request message
	if (bGeneral) {
		config_devg_request_id = 1;
		_DarkLogD("Sending DDP device general request to query device basic information\n");
		result = ddp_add_device_general_request(&ddp_winfo, config_devg_request_id);
	}
	else {
		config_devg_request_id = 2;
		_DarkLogD("Sending DDP device identity request\n");
		result = ddp_add_device_identity_request(&ddp_winfo, config_devg_request_id, NULL);
	}

	if (result != AUD_SUCCESS)
	{
		_DarkLogE("ERROR: _reqDevInfoWTF - ddp_add\n");
		return -2;
	}

	// return ddp packet length.
	ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	if (_prepare_uhip_packet_and_send(m_cobs_tx_buffer, ddp_packet_len) != AUD_SUCCESS) {
		_DarkLogE("ERROR: _reqDevInfoWTF - ddp_packet_write_get_length_bytes\n");
		return -3;
	}

	return 0;
}

int32_t Dark_Dante_UHIP::requestGeneralInfo() {
	if (m_bIsInProc) {
		_DarkLogE("ERROR: requestGeneralInfo - System is in Proccessing...\n");
		return -1;
	}
	m_nProcStep = 1;
	m_bIsInProc = true;
	return _reqDevInfo(true);
}
int32_t Dark_Dante_UHIP::requestDeviceId() {
	if (m_bIsInProc) {
		_DarkLogE("ERROR: requestDeviceId - System is in Proccessing...\n");
		return -1;
	}
	m_nProcStep = 2;
	m_bIsInProc = true;
	return _reqDevInfo(false);
}

int32_t Dark_Dante_UHIP::GetDeviceInfo(bool bDoUpdate, Dark_Dante_DevInfo** ppInfo) {
	if (m_bIsInProc) {
		uint32_t nTSP;
		if (m_pDDDI->IsUpdated()) {
			if (ppInfo != NULL) *ppInfo = m_pDDDI;
			m_bIsInProc = false;
			return 0;
		}		
		nTSP = DARKPIF_GetTickCount();
		if (nTSP > m_nTimestamp && nTSP - m_nTimestamp > 500) { //적어도 0.5초 안에는 받아야 한다
			_DarkLogE("Timeout - GetDeviceInfo - Force reset!\n");
		}
		else return 1; //Waiting...
	}

	if (!bDoUpdate && m_pDDDI->IsUpdated()) {
		if (ppInfo != NULL) *ppInfo = m_pDDDI;
		m_bIsInProc = false;
		return 0;
	}

	m_bIsInProc = true;
	m_nTimestamp = DARKPIF_GetTickCount();
	if (bDoUpdate) {
		delete m_pDDDI;
		m_pDDDI = new Dark_Dante_DevInfo();
	}
	m_nProcStep = 3;
	m_bIsInProc = true;
	_reqDevInfo(true);

	return 2; //Query
}
