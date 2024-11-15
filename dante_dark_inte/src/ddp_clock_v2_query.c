/*
* File     : ddp_clock_v2_read.c
* Created  : July 2020
* Updated  : 2020/07/10
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Clock v2 client functions to read data.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/clock_v2_query.h"
#include "../include/ddp/proto/clock_v2_structures.h"
#include "../include/ddp/support/clock_v2_read.h"

aud_error_t
ddp_clock_v2_config_read_payload
(
	const ddp_message_read_info_t * message_info,
	ddp_clock_v2_info_t *info
)
{
	if (!message_info || !info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	const ddp_raw_clock_v2_payload_t * raw_payload;

	if (ntohs(message_info->header.header->payload_length_bytes) < sizeof(*raw_payload)) {
		return AUD_ERR_INVALIDDATA;
	}
	raw_payload = (ddp_raw_clock_v2_payload_t *) message_info->payload.value32;

	info->mode_flags_mask = ntohs(raw_payload->mode_flags_mask);
	info->mode_flags = ntohs(raw_payload->mode_flags);

	info->config_flags_mask = ntohl(raw_payload->config_flags_mask);
	info->config_flags = ntohl(raw_payload->config_flags);

	info->capability_flags_mask = ntohl(raw_payload->capability_flags_mask);
	info->capability_flags = ntohl(raw_payload->capability_flags);

	info->status_flags_mask = ntohl(raw_payload->status_flags_mask);
	info->status_flags = ntohl(raw_payload->status_flags);

	info->field_flags = ntohl(raw_payload->field_flags);

	info->priority1 = raw_payload->priority1;
	info->priority2 = raw_payload->priority2;
	info->domain_number = raw_payload->domain_number;
	info->sync_interval = raw_payload->sync_interval;
	info->announce_interval = raw_payload->announce_interval;
	info->v2_ttl = raw_payload->v2_ttl;

	return AUD_SUCCESS;
}


aud_error_t
ddp_read_clock_v2_query_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_clock_v2_info_t * out_info
)
{
	aud_error_t result;
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
	result = ddp_packet_validate_message(&message_info, DDP_OP_CLOCK_V2_QUERY, sizeof(ddp_clock_v2_info_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	return ddp_clock_v2_config_read_payload(&message_info, out_info);
}

aud_error_t
ddp_add_clock_v2_query_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	ddp_message_write_info_t mwi;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	return ddp_packet_allocate_request
	(
		packet_info, &mwi,
		DDP_OP_CLOCK_V2_QUERY,
		request_id,
		0
	);
}

