/*
* File     : ddp_switch_write_client.c
* Created  : December 2019
* Updated  : Date: 2019/08/22
* Author   : Savin Weeraratne <savin.weeraratne@audinate.com>
* Synopsis : All switch register access DDP client write helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/switch_client.h"
#include "../include/ddp/proto/switch_structures.h"

aud_error_t
ddp_add_switch_reg_access_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint8_t phy_addr,
	uint8_t phy_reg,
    uint16_t phy_data,
	uint16_t rw_flag
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;
	ddp_switch_reg_access_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										   DDP_OP_SWITCH_REG_ACCESS,
										   request_id,
										   sizeof(raw_header->payload));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_switch_reg_access_request_t *)message_info._.buf32;
	raw_header->payload.phy_addr = phy_addr;
    raw_header->payload.phy_reg = phy_reg;
	raw_header->payload.phy_data = htons(phy_data);
	raw_header->payload.rw_flag = htons(rw_flag);

	return AUD_SUCCESS;
}
