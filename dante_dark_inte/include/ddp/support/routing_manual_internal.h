/*
* File     : routing_manual_internal.h
* Created  : September 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : common DDP helper functions for manual routing.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_ROUTING_MANUAL_INTERNAL__H
#define _DDP_ROUTING_MANUAL_INTERNAL__H

#include "../routing_manual_common.h"
#include "../support/routing_support.h"

aud_error_t
ddp_add_routing_manaul_tx_flow_config_request_common
(
	const ddp_manual_tx_flow_params_t * manual_tx_flow,
	ddp_routing_support_init_flow_t * flow_info
);


aud_error_t
ddp_add_routing_manual_rx_flow_config_request_common
(
	ddp_routing_support_init_flow_t * flow_info,
	ddp_manual_rx_flow_params_t * manual_rx_flow
);

#endif
