/*
 * File     : routing_manual_support.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Wrappers for routing manual handling
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */


//----------
// Include

#include "../include/ddp/support/routing_manual_support.h"
#include "../include/ddp/support/routing_support.h"

aud_error_t
ddp_routing_support_init_tx_flow_manual_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	ddp_opcode_t opcode,
	uint16_t request_id,
	dante_id_t flow_id,	// in
	uint16_t num_slots,
	uint16_t num_addrs
)
{
	assert(packet_info && flow_info);

	return init_tx_flow_common_request(
		packet_info, flow_info, request_id, opcode,
		flow_id, num_slots, num_addrs
	);
}

aud_error_t
ddp_routing_support_init_rx_flow_manual_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	ddp_opcode_t opcode, // in
	uint16_t request_id,
	dante_id_t flow_id,	// in
	uint16_t num_slots,
	uint16_t num_addrs
)
{
	aud_error_t result;

	assert(packet_info && flow_info);

	result = init_flow_request_common(
		packet_info, flow_info, request_id, opcode, flow_id, num_addrs
	);
	if (result != AUD_SUCCESS)
		return result;

	result = ddp_routing_rx_flow_slots_allocate(&flow_info->message, &flow_info->slots.rx,
		(ddp_routing_data_array_overlay_t *) &flow_info->flow->num_slots,
		num_slots
	);
	if (result != AUD_SUCCESS)
		return result;

	return AUD_SUCCESS;
}

//----------
