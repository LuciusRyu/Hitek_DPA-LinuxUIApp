/*
 * File     : routing_manual_support.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Wrappers for routing manual handling
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_ROUTING_MANUAL_SUPPORT__H
#define _DDP_ROUTING_MANUAL_SUPPORT__H


//----------
// Include

#include "../ddp_shared.h"
#include "routing_support.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------
// Flows

/*
	Input: an initialised packet, uninitialised message info
	Output: empty but initialised flow data structure
 */
aud_error_t
ddp_routing_support_init_tx_flow_manual_request
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_flow_t * flow_info,	// out
	ddp_opcode_t opcode,	//in
	uint16_t request_id,	// in
	dante_id_t flow_id,	// in
	uint16_t num_slots,	// in
	uint16_t num_addrs	// in
);

/*
	Input: an initialised packet, uninitialised message info
	Output: empty but initialised flow data structure

	Use ddp_routing_rx_flow_slots_set_channel_at_slot and
	ddp_routing_rx_flow_slots_allocate_channels_at_slot to populate slots
 */
aud_error_t
ddp_routing_support_init_rx_flow_manual_request
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_flow_t * flow_info,	// out
	ddp_opcode_t opcode,	// in
	uint16_t request_id,	// in
	dante_id_t flow_id,	// in
	uint16_t num_slots,	// in
	uint16_t num_addrs	// in
);


#ifdef __cplusplus
}
#endif

#endif // _DDP_ROUTING_MANUAL_SUPPORT__H
