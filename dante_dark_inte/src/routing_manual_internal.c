/*
* File     : routing_manual_internal.c
* Created  : November 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : common DDP helper functions for manual routing.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential.
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/support/routing_manual_internal.h"
//#include "../include/ddp/routing_manual_common.h"

aud_error_t
ddp_add_routing_manaul_tx_flow_config_request_common
(
	const ddp_manual_tx_flow_params_t * manual_tx_flow,
	ddp_routing_support_init_flow_t * flow_info
)
{
	aud_error_t result = AUD_SUCCESS;
	uint32_t i;
	result = ddp_routing_support_flow_set_label(flow_info, manual_tx_flow->label);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	flow_info->flow->flow_flags = htons(0);
	flow_info->flow->flow_flags_mask = htons(0);
	flow_info->flow->encoding = htons(manual_tx_flow->encoding);
	flow_info->flow->sample_rate = htonl(manual_tx_flow->sample_rate);
	flow_info->flow->fpp = htons(manual_tx_flow->fpp);
	flow_info->flow->latency = htonl(manual_tx_flow->latency);

	flow_info->flow->slot_size = htons(sizeof(uint16_t));
	flow_info->flow->addr_size = htons(sizeof(ddp_addr_inet_t));
	flow_info->flow->num_addresses = htons(manual_tx_flow->num_addr);
	flow_info->flow->num_slots = htons(manual_tx_flow->num_slots);

	for (i = 0; i < manual_tx_flow->num_slots; ++i)
	{
		flow_info->slots.tx[i] = htons(manual_tx_flow->slots[i]);
	}

	for (i = 0; i < manual_tx_flow->num_addr; ++i)
	{
		ddp_addr_inet_init(&flow_info->addrs[i], manual_tx_flow->addrs[i].ip_addr, manual_tx_flow->addrs[i].port);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_manual_rx_flow_config_request_common
(
	ddp_routing_support_init_flow_t * flow_info,
	ddp_manual_rx_flow_params_t * manual_rx_flow
)
{
	aud_error_t result = AUD_SUCCESS;
	uint32_t i;

	result = ddp_routing_support_flow_set_label(flow_info, manual_rx_flow->label);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	flow_info->flow->encoding = htons(manual_rx_flow->encoding);
	flow_info->flow->sample_rate = htonl(manual_rx_flow->sample_rate);
	flow_info->flow->fpp = htons(manual_rx_flow->fpp);
	flow_info->flow->latency = htonl(manual_rx_flow->latency);
	flow_info->flow->flow_flags = 0;
	flow_info->flow->flow_flags_mask = 0;
	flow_info->flow->slot_size = htons(sizeof(uint16_t));
	flow_info->flow->addr_size = htons(sizeof(ddp_addr_inet_t));
	flow_info->flow->num_addresses = htons(manual_rx_flow->num_addr);
	flow_info->flow->num_slots = htons(manual_rx_flow->num_slots);

	for (i = 0; i < manual_rx_flow->num_slots; ++i)
	{
		if (manual_rx_flow->slot_map[i].num_chans <= 1)
		{
			ddp_routing_rx_flow_slots_set_channel_at_slot(&flow_info->slots.rx, manual_rx_flow->slot_map[i].slot_id - 1, manual_rx_flow->slot_map[i].chans[0]);
		}
		else
		{
			uint16_t * channels = ddp_routing_rx_flow_slots_allocate_channels_at_slot(
				&flow_info->slots.rx, manual_rx_flow->slot_map[i].slot_id - 1, manual_rx_flow->slot_map[i].num_chans);

			uint32_t j;
			for (j = 0; j < manual_rx_flow->slot_map[i].num_chans; ++j)
			{
				channels[j] = htons(manual_rx_flow->slot_map[i].chans[j]);
			}
		}
	}

	for (i = 0; i < manual_rx_flow->num_addr; ++i)
	{
		ddp_addr_inet_init(&flow_info->addrs[i], manual_rx_flow->addrs[i].ip_addr, manual_rx_flow->addrs[i].port);
	}
	return AUD_SUCCESS;
}
