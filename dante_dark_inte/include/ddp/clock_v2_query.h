/*
* File     : clock_v2_query.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Client side functions to query and read DDP Clock v2
*
* This software is Copyright (c) 2004-2016, Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_CLOCK_V2_QUERY__H
#define _DDP_CLOCK_V2_QUERY__H

//----------
// Include

#include "packet.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
* @file clock_v2_query.h
* @brief DDP Clock v2 client helper functions to query and read response.
*/

/**
* \addtogroup DDP
* @{
*/

/**
* \addtogroup Clocking
* @{
*/

/**
* \addtogroup ClockingStructs
* @{
*/

struct ddp_clock_v2_info
{
	uint16_t mode_flags;		//!< Clock v2 mode flags @see ddp_clock_v2_mode_flags
	uint16_t mode_flags_mask;	//!< mask for mode flags
	uint32_t config_flags;		//!< Config flags @see ddp_clock_v2_config_flags
	uint32_t config_flags_mask;	//!< mask for config flags
	uint32_t capability_flags;	//!< Currently none defined use zero - Ignored on requests
	uint32_t capability_flags_mask;	//!< mask for capability_flags
	uint32_t status_flags;		//!< None defined yet use zero. Ignored on requests
	uint32_t status_flags_mask;	//!< mask for status_flags
	uint32_t field_flags;		//!< Field flags for this message @see ddp_clock_v2_field_flags
	uint8_t priority1;		//!< Priority1 field of PTPv2
	uint8_t priority2;		//!< Priority2 field of PTPv2
	uint8_t domain_number;		//!< domain number field of PTPv2
	uint8_t v2_ttl;			//!< PTP v2 multicast port TTL value
	int8_t sync_interval;		//!< PTP v2 sync interval (value is log to the base 2)
	int8_t announce_interval;	//!< PTP v2 announce interval (value is log to the base 2)
};

//! @cond Doxygen_Suppress
typedef struct ddp_clock_v2_info ddp_clock_v2_info_t;
//! @endcond

/**@}*/

/**
* \addtogroup ClockFuncs
* @{
*/

/**
* Read data fields from a DDP Clock v2 query response message.
*
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] offset Starting point in the packet buffer to read this message
* @param[out] info Pointer to client clock v2 data structure (memory should be allocated by the caller of this function)
* @return AUD_SUCCESS if the DDP clock v2 config or query response message was successfully read, else a non AUD_SUCCESS value
*	is returned
*/
aud_error_t
ddp_read_clock_v2_query_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_clock_v2_info_t * out_info
);

/**
* Creates a DDP clock v2 request message to query the device state. This message does not change the state of the device.
*
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] request_id A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP RTP audio config request message was successfully created, else a non AUD_SUCCESS
*	value is returned
*/
aud_error_t
ddp_add_clock_v2_query_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif //

