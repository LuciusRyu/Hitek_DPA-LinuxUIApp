/*
* File     : rtp_audio_read.c
* Created  : July 2020
* Updated  : 2020/07/10
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : DDP RTP audio client functions to read data.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/proto/rtp_audio_structures.h"
#include "../include/ddp/rtp_audio_query.h"
#include "../include/ddp/support/rtp_audio_read.h"


aud_error_t
ddp_audio_rtp_config_read_payload
(
	const ddp_message_read_info_t * message_info,
	ddp_audio_rtp_info_t *info
)
{
	if (!message_info || !info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	const ddp_raw_audio_rtp_routing_payload_t * raw_payload;

	if (ntohs(message_info->header.header->payload_length_bytes) < sizeof(*raw_payload)) {
		return AUD_ERR_INVALIDDATA;
	}
	raw_payload = (ddp_raw_audio_rtp_routing_payload_t *)message_info->payload.value32;

	info->config_flags_mask = ntohl(raw_payload->config_flags_mask);
	info->config_flags = ntohl(raw_payload->config_flags);

	info->capability_flags_mask = ntohl(raw_payload->capability_flags_mask);
	info->capability_flags = ntohl(raw_payload->capability_flags);

	info->status_flags_mask = ntohl(raw_payload->status_flags_mask);
	info->status_flags = ntohl(raw_payload->status_flags);

	info->field_flags = ntohl(raw_payload->field_flags);

	if (AUD_FLAG_ISSET(info->field_flags, DDP_AUDIO_RTP_FIELD_FLAG__ADDRESS_PREFIX_VALID))
		info->address_prefix = raw_payload->address_prefix;
	if (AUD_FLAG_ISSET(info->field_flags, DDP_AUDIO_RTP_FIELD_FLAG__DESTINATION_PORT_VALID))
		info->destination_port = ntohs(raw_payload->destination_port);
	if (AUD_FLAG_ISSET(info->field_flags, DDP_AUDIO_RTP_FIELD_FLAG__PACKET_TIME_VALID))
		info->fpp = ntohs(raw_payload->fpp);
	if (AUD_FLAG_ISSET(info->field_flags, DDP_AUDIO_RTP_FIELD_FLAG__LATENCY_VALID))
		info->latency = ntohl(raw_payload->latency);

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_rtp_query_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_audio_rtp_info_t * out_info
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
	result = ddp_packet_validate_message(&message_info, DDP_OP_AUDIO_RTP_QUERY, sizeof(ddp_audio_rtp_info_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	return ddp_audio_rtp_config_read_payload(&message_info, out_info);
}

aud_error_t
ddp_add_audio_rtp_query_request
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
		packet_info,
		&mwi,
		DDP_OP_AUDIO_RTP_QUERY,
		request_id,
		0
	);
}
