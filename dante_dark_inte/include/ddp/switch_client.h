/*
 * File     : switch_client.h
 * Created  : December 2019
 * Updated  : Date: 2019/12/11
 * Author   : Savin Weeraratne <savin.weeraratne@audinate.com>
 * Synopsis : All switch register access DDP client helper functions.
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef _DDP_SWITCH_CLIENT_H
#define _DDP_SWITCH_CLIENT_H

#include "packet.h"
#include "proto/switch_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file switch_client.h
* @brief All switch register access DDP client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
 * \defgroup Switch DDP Switch
 * @{
 */

/**
* \defgroup SwitchFuncs DDP Switch Functions
* @{
*/

/**
* Creates a DDP switch register access request message. This message can be used to read from or write a value to a switch register
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param phy_addr [in] phy_id subjected to read or write operation
* @param phy_reg [in] phy_register subjected to read or write operation
* @param phy_data [in] data sent to the phy for write operation
* @param rw_flag [in] Flag to specify read or write operation @see switch_access_operation
* @return AUD_SUCCESS if the DDP switch register access request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_switch_reg_access_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
    uint8_t phy_addr,
    uint8_t phy_reg,
	uint16_t phy_data,
	uint16_t rw_flag
);

/**
* Reads data fields from a DDP switch register access response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_register_value [out optional] Pointer to the register value carried by this message
* @return AUD_SUCCESS if the DDP switch register access response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_switch_reg_access_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_register_value
);

/**@}*/
/**@}*/
/**@}*/

#endif /* _DDP_SWITCH_CLIENT_H */
