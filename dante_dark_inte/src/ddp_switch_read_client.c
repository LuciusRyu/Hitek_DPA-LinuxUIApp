/*
* File     : ddp_switch_read_client.c
* Created  : December 2019
* Updated  : Date: 2019/08/22
* Author   : Savin Weeraratne <savin.weeraratne@audinate.com>
* Synopsis : All switch register access DDP client read helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/switch_client.h"
#include "../include/ddp/proto/switch_structures.h"

aud_error_t
ddp_read_switch_reg_access_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_register_value
)
{
	aud_error_t result;
	ddp_switch_reg_access_response_t * header;
	ddp_message_read_info_t message_info;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_SWITCH_REG_ACCESS, sizeof(ddp_switch_reg_access_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_switch_reg_access_response_t *)message_info.header.header;
	
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}

	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_register_value)
	{
		*out_register_value = ntohs(header->payload.register_value);
	}

	return AUD_SUCCESS;
}
