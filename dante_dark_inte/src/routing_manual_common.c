/*
* File     : routing_manual_params.h
* Created  : September 2014
* Author   : Chamira Perera <cperera@audinate.com>
* Synopsis : routing manual/RTP DDP common client.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/routing_manual_common.h"

void
ddp_routing_manual_tx_flow_config_add_slot_params
(
	ddp_manual_tx_flow_params_t * manual_tx_flow,
	uint16_t * slots
)
{
	manual_tx_flow->slots = slots;
}

void
ddp_routing_manual_rx_flow_config_add_slot_map
(
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rx_slots_t * slot_map
)
{
	manual_rx_flow->slot_map = slot_map;
}

void
ddp_routing_manual_rx_flow_config_add_slot_params
(
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	uint16_t slot_idx,
	uint16_t slot_id,
	uint16_t num_chans,
	uint16_t * chans
)
{
	manual_rx_flow->slot_map[slot_idx].slot_id = slot_id;
	manual_rx_flow->slot_map[slot_idx].num_chans = num_chans;
	manual_rx_flow->slot_map[slot_idx].chans = chans;
}
