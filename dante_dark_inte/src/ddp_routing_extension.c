/*
* File     : ddp_routing_client_extension.c
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : DDP routing client helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/routing_extension.h"
#include "../include/ddp/proto/routing_rtp_structures.h"
#include "../include/ddp/ddp_shared.h"
#include "../include/ddp/support/routing_support.h"
#include "../include/ddp/support/routing_manual_support.h"
#include "../include/ddp/support/routing_manual_internal.h"
#include "../include/ddp/support/routing_rtp_support.h"

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_params_with_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_rx_flow_params_with_extension_t * out_rx_flow
)
{
	aud_error_t result;
	ddp_routing_flow_data_t *fd = NULL;

	if (!message_info || !out_rx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_get_raw_flow_state_data_ptr(message_info, flow_idx, &fd);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	out_rx_flow->flow_id = ntohs(fd->flow_id);

	if (fd->label_offset) {
		out_rx_flow->flow_label = ddp_heap_read_string(message_info, ntohs(fd->label_offset));
	}
	else {
		out_rx_flow->flow_label = NULL;
	}

	out_rx_flow->status = ntohs(fd->status);
	out_rx_flow->flow_flags = ntohs(fd->flow_flags);
	out_rx_flow->flow_flags_mask = ntohs(fd->flow_flags_mask);
	out_rx_flow->user_conf_flags = ntohl(fd->user_conf_flags);
	out_rx_flow->user_conf_mask = ntohl(fd->user_conf_mask);
	out_rx_flow->transport = fd->transport;
	out_rx_flow->avail_mask = fd->avail_mask;
	out_rx_flow->active_mask = fd->active_mask;
	out_rx_flow->sample_rate = ntohl(fd->sample_rate);
	out_rx_flow->latency = ntohl(fd->latency) / 1000;
	out_rx_flow->encoding = ntohs(fd->encoding);
	out_rx_flow->fpp = ntohs(fd->fpp);

	out_rx_flow->num_slots = ntohs(fd->num_slots);
	out_rx_flow->num_addresses = ntohs(fd->num_addresses);
	if (AUD_FLAG_ISSET(fd->extension_flags, ROUTING_FLOW_DATA_EXTENSION_FLAG__TRANSPORT) &&
		out_rx_flow->transport == DDP_AUDIO_TRANSPORT_RTP)
	{
		AUD_FLAG_SET(out_rx_flow->flow_state_flags, DDP_FLOW_STATE_FLAG__RTP_AUDIO_DATA);
	}

	return AUD_SUCCESS;
}


aud_error_t
ddp_read_routing_flow_config_state_response_rtp_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_rtp_audio_flow_params_t * out_rtp_params
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	ddp_routing_flow_data_t *fd = NULL;
	ddp_raw_rtp_flow_extension_data_t* raw_extension = NULL;

	if (!message_info || !out_rtp_params)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_get_raw_flow_state_data_ptr(message_info, flow_idx, &fd);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	uint16_t extension_size = ntohs(fd->transport_extension_size);
	if (!AUD_FLAG_ISSET(fd->extension_flags, ROUTING_FLOW_DATA_EXTENSION_FLAG__TRANSPORT))
	{
		return AUD_ERR_INVALIDDATA;
	}
	if (extension_size < AUD_OFFSETOF_END(*raw_extension, source_address))
	{
		return AUD_ERR_INVALIDDATA;
	}
	if (ddp_read_heap_arraystruct
			(
				message_info,
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

	bzero(out_rtp_params, sizeof(*out_rtp_params));

	out_rtp_params->field_flags = ntohl(raw_extension->field_flags);

	if (AUD_FLAG_ISSET(out_rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_VERSION))
	{
		out_rtp_params->sdp_session_version_hi = ntohl(raw_extension->sdp_session_ver_hi);
		out_rtp_params->sdp_session_version_lo = ntohl(raw_extension->sdp_session_ver_lo);
	}
	if (AUD_FLAG_ISSET(out_rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID))
	{
		out_rtp_params->sdp_session_id_hi = ntohl(raw_extension->sdp_session_id_hi);
		out_rtp_params->sdp_session_id_lo = ntohl(raw_extension->sdp_session_id_lo);
	}
	if (AUD_FLAG_ISSET(out_rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET))
	{
		out_rtp_params->clock_offset = ntohl(raw_extension->clock_offset);
	}
	if (AUD_FLAG_ISSET(out_rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS))
	{
		out_rtp_params->source_address.ip_addr = raw_extension->source_address.ip_addr;
	}

	return AUD_SUCCESS;
}


aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_params_with_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_tx_flow_params_with_extension_t * out_tx_flow
)
{
	aud_error_t result;
	ddp_routing_flow_data_t *fd = NULL;

	if (!message_info || !out_tx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_get_raw_flow_state_data_ptr(message_info, flow_idx, &fd);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	out_tx_flow->flow_id = ntohs(fd->flow_id);

	if (fd->label_offset) {
		out_tx_flow->flow_label = ddp_heap_read_string(message_info, ntohs(fd->label_offset));
	}
	else {
		out_tx_flow->flow_label = NULL;
	}

	out_tx_flow->status = ntohs(fd->status);
	out_tx_flow->flow_flags = ntohs(fd->flow_flags);
	out_tx_flow->flow_flags_mask = ntohs(fd->flow_flags_mask);
	out_tx_flow->user_conf_flags = ntohl(fd->user_conf_flags);
	out_tx_flow->user_conf_mask = ntohl(fd->user_conf_mask);
	out_tx_flow->peer_flow_id = ntohs(fd->peer_flow_id);

	if (fd->peer_flow_label_offset) {
		out_tx_flow->peer_flow_label = ddp_heap_read_string(message_info, ntohs(fd->peer_flow_label_offset));
	}
	else {
		out_tx_flow->peer_flow_label = NULL;
	}

	memcpy(out_tx_flow->peer_device_id, fd->peer_device_id, sizeof(out_tx_flow->peer_device_id));
	out_tx_flow->peer_process_id = ntohs(fd->peer_process_id);

	if (fd->peer_device_label_offset) {
		out_tx_flow->peer_device_label = ddp_heap_read_string(message_info, ntohs(fd->peer_device_label_offset));
	}
	else {
		out_tx_flow->peer_device_label = NULL;
	}

	out_tx_flow->transport = fd->transport;
	out_tx_flow->avail_mask = fd->avail_mask;
	out_tx_flow->active_mask = fd->active_mask;
	out_tx_flow->sample_rate = ntohl(fd->sample_rate);
	out_tx_flow->latency = ntohl(fd->latency) / 1000;
	out_tx_flow->encoding = ntohs(fd->encoding);
	out_tx_flow->fpp = ntohs(fd->fpp);
	out_tx_flow->num_slots = ntohs(fd->num_slots);
	out_tx_flow->num_addresses = ntohs(fd->num_addresses);
	if (AUD_FLAG_ISSET(fd->extension_flags, ROUTING_FLOW_DATA_EXTENSION_FLAG__TRANSPORT) &&
		out_tx_flow->transport == DDP_AUDIO_TRANSPORT_RTP)
	{
		AUD_FLAG_SET(out_tx_flow->flow_state_flags, DDP_FLOW_STATE_FLAG__RTP_AUDIO_DATA);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rtp_tx_flow_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_tx_flow_params_t * manual_tx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_flow_t flow_info;

	if (!packet_info || !manual_tx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_tx_flow_manual_request(packet_info,
		&flow_info,
		DDP_OP_ROUTING_RTP_TX_FLOW_CONFIG_SET,
		request_id,
		manual_tx_flow->flow_id,
		manual_tx_flow->num_slots,
		manual_tx_flow->num_addr
	);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_add_routing_manaul_tx_flow_config_request_common(manual_tx_flow, &flow_info);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	flow_info.flow->transport = DDP_AUDIO_TRANSPORT_RTP;

	if (AUD_FLAG_ISSET(rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID))
	{
		ddp_util_aes67_flow_info_t info = {0};
		info.sdp_session_id = rtp_params->sdp_session_id_hi;
		info.sdp_session_id <<= 32;
		info.sdp_session_id |= rtp_params->sdp_session_id_lo;
		AUD_FLAG_SET(info.field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID);

		result = ddp_util_add_rtp_flow_data_extension(&flow_info.message, flow_info.flow, &info);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}
	return AUD_SUCCESS;
}


aud_error_t
ddp_add_routing_rtp_rx_flow_manual_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_flow_t flow_info;

	if (!packet_info || !manual_rx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_rx_flow_manual_request(packet_info,
		&flow_info,
		DDP_OP_ROUTING_RTP_RX_FLOW_CONFIG_SET,
		request_id,
		manual_rx_flow->flow_id,
		manual_rx_flow->num_slots,
		manual_rx_flow->num_addr
	);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_add_routing_manual_rx_flow_config_request_common(&flow_info, manual_rx_flow);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_FLAG_SET(flow_info.flow->flow_flags_mask, htons(DDP_ROUTING_RXFLOW_FLAG__FLOWSUB));

	flow_info.flow->transport = DDP_AUDIO_TRANSPORT_RTP;

	if (AUD_FLAG_ISSET(rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET))
	{
		ddp_util_aes67_flow_info_t info = { 0 };
		info.media_clock_offset = rtp_params->clock_offset;
		AUD_FLAG_SET(info.field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET);

		result = ddp_util_add_rtp_flow_data_extension(&flow_info.message, flow_info.flow, &info);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rtp_rx_flow_sdp_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_flow_t flow_info;

	if (!packet_info || !manual_rx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_rx_flow_manual_request(packet_info,
		&flow_info,
		DDP_OP_ROUTING_RTP_RX_FLOW_CONFIG_SET,
		request_id,
		manual_rx_flow->flow_id,
		manual_rx_flow->num_slots,
		manual_rx_flow->num_addr
	);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_add_routing_manual_rx_flow_config_request_common(&flow_info, manual_rx_flow);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_FLAG_SET(flow_info.flow->flow_flags, htons(DDP_ROUTING_RXFLOW_FLAG__FLOWSUB));
	AUD_FLAG_SET(flow_info.flow->flow_flags_mask, htons(DDP_ROUTING_RXFLOW_FLAG__FLOWSUB));

	flow_info.flow->transport = DDP_AUDIO_TRANSPORT_RTP;
	if (rtp_params->field_flags)
	{
		ddp_util_aes67_flow_info_t info = { 0 };
		if (AUD_FLAG_ISSET(rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID))
		{
			info.sdp_session_id = rtp_params->sdp_session_id_hi;
			info.sdp_session_id <<= 32;
			info.sdp_session_id |= rtp_params->sdp_session_id_lo;
			AUD_FLAG_SET(info.field_flags, RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID);
		}
		if (AUD_FLAG_ISSET(rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET))
		{
			info.media_clock_offset = rtp_params->clock_offset;
			AUD_FLAG_SET(info.field_flags, RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET);
		}
		if (AUD_FLAG_ISSET(rtp_params->field_flags, RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS))
		{
			info.sap_origin_addr.ip_addr = rtp_params->source_address.ip_addr;
			AUD_FLAG_SET(info.field_flags, RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS);
		}

		result = ddp_util_add_rtp_flow_data_extension(&flow_info.message, flow_info.flow, &info);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}
	return AUD_SUCCESS;
}
