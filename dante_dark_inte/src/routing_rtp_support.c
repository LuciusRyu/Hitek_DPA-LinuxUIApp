/*
* File     : routing_rtp_support.c
* Created  : September 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : RTP routing extension utility definitions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/support/routing_rtp_support.h"
#include "../include/ddp/proto/routing_rtp_structures.h"


//read rtp extension data from flow config request
aud_error_t
ddp_util_read_rtp_flow_data_extension
(
	const ddp_message_read_info_t * mri,
	const ddp_routing_flow_data_t * fd,
	ddp_util_aes67_flow_info_t * info
)
{
	uint16_t extension_size = ntohs(fd->transport_extension_size);
	ddp_raw_rtp_flow_extension_data_t* raw_extension = NULL;
	ddp_heap_read_info_t heap;

	if (!AUD_FLAG_ISSET(fd->extension_flags, ROUTING_FLOW_DATA_EXTENSION_FLAG__TRANSPORT))
	{
		return AUD_ERR_INVALIDDATA;
	}
	if (extension_size < AUD_OFFSETOF_END(*raw_extension, source_address))//sizeof(ddp_raw_rtp_flow_extension_data_t))
	{
		return AUD_ERR_INVALIDDATA;
	}
	if (
		ddp_read_heap_arraystruct
		(
			mri,
			&heap,
			ntohs(fd->transport_extension_offset),
			1,
			extension_size
		) != AUD_SUCCESS
	)
	{
		return AUD_ERR_INVALIDDATA;
	}
	raw_extension = (ddp_raw_rtp_flow_extension_data_t*)heap._.array8;
	info->field_flags = ntohl(raw_extension->field_flags);
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_VERSION))
	{
		uint64_t session_version = ntohl(raw_extension->sdp_session_ver_hi);
		session_version <<= 32;
		session_version |= ntohl(raw_extension->sdp_session_ver_lo);
		info->sdp_session_version = session_version;
	}
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID))
	{
		uint64_t session_id = ntohl(raw_extension->sdp_session_id_hi);
		session_id <<= 32;
		session_id |= ntohl(raw_extension->sdp_session_id_lo);
		info->sdp_session_id = session_id;
	}
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET))
	{
		info->media_clock_offset = ntohl(raw_extension->clock_offset);
	}
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS))
	{
		info->sap_origin_addr.ip_addr = raw_extension->source_address.ip_addr;
	}

	return AUD_SUCCESS;
}

// add rtp info to raw flow data
aud_error_t
ddp_util_add_rtp_flow_data_extension
(
	ddp_message_write_info_t * mwi,
	ddp_routing_flow_data_t * fd,
	const ddp_util_aes67_flow_info_t * info
)
{
	ddp_raw_rtp_flow_extension_data_t rtp_raw_data = { 0 };

	if (!info->field_flags)
		return AUD_ERR_INVALIDDATA;

	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS))
	{
		rtp_raw_data.source_address.ip_addr = info->sap_origin_addr.ip_addr;
	}
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET))
	{
		rtp_raw_data.clock_offset = htonl(info->media_clock_offset);
	}
	//session id and session version are 64-bit
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID))
	{
		//session id
		uint32_t hi = info->sdp_session_id >> 32;
		uint32_t lo = info->sdp_session_id & UINT32_MAX;
		rtp_raw_data.sdp_session_id_hi = htonl(hi);
		rtp_raw_data.sdp_session_id_lo = htonl(lo);
	}
	if (AUD_FLAG_ISSET(info->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_VERSION))
	{
		//session version
		uint32_t hi = info->sdp_session_version >> 32;
		uint32_t lo = info->sdp_session_version & UINT32_MAX;
		rtp_raw_data.sdp_session_ver_hi = htonl(hi);
		rtp_raw_data.sdp_session_ver_lo = htonl(lo);
	}

	rtp_raw_data.field_flags = htonl(info->field_flags);

	if (
		!ddp_message_allocate_data(mwi,
			&fd->transport_extension_offset,
			sizeof(ddp_raw_rtp_flow_extension_data_t),
			&rtp_raw_data)
		)
	{
		return AUD_ERR_NOMEMORY;
	}

	fd->transport_extension_size = htons(sizeof(ddp_raw_rtp_flow_extension_data_t));
	fd->extension_flags = ROUTING_FLOW_DATA_EXTENSION_FLAG__TRANSPORT;
	return AUD_SUCCESS;
}
