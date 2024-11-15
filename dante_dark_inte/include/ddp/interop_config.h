/*
* File     : interop_config.h
* Created  : September 2020
* Updated  : Date: 2020/09/30
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Interop config helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_INTEROP_CONFIG_H
#define _DDP_INTEROP_CONFIG_H

#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file interop_config.h
* @brief DDP interop client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
* \addtogroup Device
* @{
*/

/**
* \addtogroup DeviceStructs
* @{
*/

/**
* @struct ddp_interop_params
* @brief holds Interop config parameters
*/
struct ddp_interop_params
{
	uint16_t interop_action_flags;		/*!< interop action flags @see device_interop_action_flags */
	uint16_t interop_action_flags_mask;	/*!< mask for interop flags */
};

//! @cond Doxygen_Suppress
typedef struct ddp_interop_params ddp_interop_params_t;
//! @endcond

/**@}*/

/**
* \addtogroup DeviceFuncs
* @{
*/

/**
* Creates a DDP Interop config request message.
* @param [in]packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param [in]request_id [in] A non-zero sequence number which is populated in this message
* @param [in]params Pointer to a structure that contains the config parameters.
*/
aud_error_t
ddp_add_interop_config
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_interop_params_t * params
);

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_INTEROP_CONFIG_H

