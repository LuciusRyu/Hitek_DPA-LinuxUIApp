/*
 * File     : rtp_structures.h
 * Created  : July 2020
 * Author   : Naeem Bacha <naeem.bacha@audinate.com>
 * Synopsis : All rtp DDP messages structure definitions.
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_RTP_AUDIO_STRUCTURES_H
#define _DDP_RTP_AUDIO_STRUCTURES_H


//----------

#include "../ddp.h"

#include "../shared_structures.h"

/**
* @file rtp_structures.h
* @brief All rtp DDP messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Audio
*  @{
*/

/**
* \addtogroup AudioEnums
* @{
*/

/**
* @enum ddp_audio_rtp_field_flags.
* valid flags for the field_flags in the ddp_raw_audio_rtp_routing_payload
* These flags only exist if the payload is a request.
*/
enum
{
	DDP_AUDIO_RTP_FIELD_FLAG__ADDRESS_PREFIX_VALID = 1 << 0,
	DDP_AUDIO_RTP_FIELD_FLAG__DESTINATION_PORT_VALID = 1 << 1,
	DDP_AUDIO_RTP_FIELD_FLAG__PACKET_TIME_VALID = 1 << 2,
	DDP_AUDIO_RTP_FIELD_FLAG__LATENCY_VALID = 1 << 3,
};

/**
* @enum ddp_audio_rtp_config_flags.
* config flags in the ddp_raw_audio_rtp_routing_payload
* These flags only exist if the payload is a request.
*/
enum
{
	DDP_RAW_AUDIO_RTP_CONFIG_FLAG__ENABLE_RTP = 1 << 0,
};

/**@}*/

/**
 * \addtogroup AudioStructs
 * @{
 */

/**
* @struct ddp_raw_audio_rtp_routing_payload
* @brief Structure format for the payload of the "rtp audio routing" message.
* This payload is the same for the request and response.
*/
struct ddp_raw_audio_rtp_routing_payload
{
	uint32_t config_flags;			/*!< Config flags @see ddp_audio_rtp_config_flags */
	uint32_t config_flags_mask;		/*!< mask for config flags */
	uint32_t capability_flags;		/*!< Currently none defined use zero - Ignored on requests */
	uint32_t capability_flags_mask;	/*!< mask for capability_flags */
	uint32_t status_flags;			/*!< Status flags @see . None defined yet. Ignored on requests */
	uint32_t status_flags_mask;		/*!< mask for status_flags */
	uint32_t field_flags;			/*!< Field flags for this message @see ddp_audio_rtp_field_flags */
	uint32_t address_prefix;		/*!< IPv4 Address Prefix */
	uint16_t destination_port;		/*!< RTP flow destination port */
	uint16_t fpp;					/*!< Frames per packet / RTP transmit flow packet time */
	uint32_t latency;				/*!< RTP receive flow latency */
};

//! @cond Doxygen_Suppress
typedef struct ddp_raw_audio_rtp_routing_payload ddp_raw_audio_rtp_routing_payload_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif

