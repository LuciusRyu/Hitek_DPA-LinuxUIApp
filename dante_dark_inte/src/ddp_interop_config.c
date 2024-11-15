/*
* File     : ddp_interop_config.c
* Created  : September 2020
* Updated  : Date: 2020/09/30
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Interop config helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/interop_config.h"
#include "../include/ddp/proto/device_structures.h"

aud_error_t
ddp_add_interop_config
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_interop_params_t * params
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_aes67_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_AES67, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_aes67_request_t *)message_info._.buf32;

	raw_header->payload.valid_flags = 0;
	raw_header->payload.aes67_mode = 0;
	raw_header->payload.pad0 = 0;

	raw_header->payload.interop_action_flags_mask = htons(params->interop_action_flags_mask);
	raw_header->payload.interop_action_flags = htons(params->interop_action_flags);

	return AUD_SUCCESS;
}

