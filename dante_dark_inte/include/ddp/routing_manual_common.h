/*
* File     : routing_manual_common.h
* Created  : September 2014
* Author   : Chamira Perera <cperera@audinate.com>
* Synopsis : routing manual/RTP DDP common client..
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/
#ifndef _DDP_ROUTING_MANUAL_COMMON__H
#define _DDP_ROUTING_MANUAL_COMMON__H

#include "ddp.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
* @brief Structure used to set the IP address and port for manual Tx flow configuration parameters
*/
typedef struct ddp_flow_addr
{
	uint32_t ip_addr;			/*!< IP address of the flow, only IPv4 addresses are supported */
	uint16_t port;				/*!< UDP/IP port of the flow */
} ddp_flow_addr_t;

/**
* @brief Structure used to set the manual Tx flow configuration parameters
*/
typedef struct ddp_manual_tx_flow_params
{
	uint16_t flow_id;						/*!< Non-zero ID of this flow */
	char * label;							/*!< Null-terminated flow label string */
	uint32_t sample_rate;					/*!< Audio sample rate used for this flow */
	uint32_t latency;						/*!< Audio latency in microseconds used for this flow */
	uint16_t encoding;						/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;							/*!< Frames per packet used for this flow */
	uint16_t num_slots;						/*!< Number of audio transmit channels for this flow */
	uint16_t * slots;						/*!< Array of audio transmit channels for this flow */
	uint16_t num_addr;						/*!< Number of IP addresses and ports structures */
	ddp_flow_addr_t * addrs;				/*!< Array of structures @see ddp_flow_addr_t which contain IP addresses and ports associated with this flow */
} ddp_manual_tx_flow_params_t;

/**
* @brief Structure used to set the Rx channels for each slot for a manual Rx flow
*/
typedef struct ddp_rx_slots
{
	uint16_t slot_id;		/*!< ID of the slot */
	uint16_t num_chans;		/*!< Number of Rx channels allocated to this slot */
	uint16_t * chans;		/*!< The array of channels allocated to this slot */
} ddp_rx_slots_t;

/**
* @brief Structure used to set the manual Rx flow configuration parameters
*/
typedef struct ddp_manual_rx_flow_params
{
	uint16_t flow_id;						/*!< Non-zero ID of this flow */
	char * label;							/*!< Null-terminated flow label string */
	uint32_t sample_rate;					/*!< Audio sample rate used for this flow */
	uint32_t latency;						/*!< Audio latency in microseconds used for this flow */
	uint16_t encoding;						/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;							/*!< Frames per packet used for this flow */
	uint16_t num_slots;						/*!< Number of audio transmit channels for this flow */
	ddp_rx_slots_t * slot_map;				/*!< Contains information about slots and channels allocated to each slot @see ddp_rx_slots_t */
	uint16_t num_addr;						/*!< Number of IP addresses and ports structures */
	ddp_flow_addr_t * addrs;				/*!< Array of structures @see ddp_flow_addr_t which contain IP addresses and ports associated with this flow */
} ddp_manual_rx_flow_params_t;


/**
* Adds an array of slots to the manual tx flow config parameters structure which will be passed into the ddp_add_routing_manual_tx_flow_config_request function
* @param manual_tx_flow [in] Pointer to a structure which has parameters related to how to create the manual Tx flow (memory should be allocated for this structure by the caller of this function) @see ddp_manual_tx_flow_params_t
* @param slots [in] Pointer to an array of slots which will be used to build the manual Tx flow config message (memory should be allocated for this structure by the caller of this function)
*/
void
ddp_routing_manual_tx_flow_config_add_slot_params
(
	ddp_manual_tx_flow_params_t * manual_tx_flow,
	uint16_t * slots
);

/**
* Adds an array of slot map structures @see ddp_rx_slots_t to the manual rx flow config parameters structure which will be passed into the ddp_add_routing_manual_rx_flow_config_request function
* @param manual_rx_flow [in] Pointer to a structure which has parameters related to how to create the manual Rx flow (memory should be allocated for this structure by the caller of this function) @see ddp_manual_rx_flow_params_t
* @param slot_map [in] Pointer to an array of slot map structures which will be used to build the manual Rx flow config message (memory should be allocated for this array by the caller of this function)
*/
void
ddp_routing_manual_rx_flow_config_add_slot_map
(
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rx_slots_t * slot_map
);


/**
* Populates the slot related parameters for a particular slot to the manual rx flow config parameters structure which will be passed into the ddp_add_routing_manual_rx_flow_config_request function
* @param manual_rx_flow [in] Pointer to a structure which has parameters related to how to create the manual Rx flow (memory should be allocated for this structure by the caller of this function) @see ddp_manual_rx_flow_params_t
* @param slot_idx [in] Index to the collection of slot map structures which was allocated using ddp_routing_manual_rx_flow_config_add_slot_map
* @param slot_id [in] ID for this slot
* @param num_chans [in] Number of Rx channels for this slot
* @param chans [in] Array of channels associated with this slot (memory should be allocated for this array by the caller of this function)
*/
void
ddp_routing_manual_rx_flow_config_add_slot_params
(
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	uint16_t slot_idx,
	uint16_t slot_id,
	uint16_t num_chans,
	uint16_t * chans
);

#ifdef __cplusplus
}
#endif

#endif
