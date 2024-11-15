/*
* File     : rtp_audio_query.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Client side functions to query and read DDP RTP Audio specific config.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/
#ifndef _DDP_RTP_AUDIO_QUERY_H
#define _DDP_RTP_AUDIO_QUERY_H


//----------
// Include

#include "ddp.h"
#include "packet.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
* @file rtp_audio_query.h
* @brief RTP audio DDP client helper functions to query and read data.
*/

/**
* \addtogroup DDP
* @{
*/

/**
* \addtogroup Audio
* @{
*/

/**
* \addtogroup AudioStructs
* @{
*/

/**
* @struct ddp_audio_rtp_info
* @brief Structure which contains data from the RTP query response
*/
struct ddp_audio_rtp_info
{
	uint32_t config_flags;				/*!< Config flags @see ddp_audio_rtp_config_flags */
	uint32_t config_flags_mask;			/*!< mask for config flags */
	uint32_t capability_flags;			/*!< Currently none defined use zero - Ignored on requests */
	uint32_t capability_flags_mask;		/*!< mask for capability_flags */
	uint32_t status_flags;				/*!< None defined yet use zero. Ignored on requests */
	uint32_t status_flags_mask;			/*!< mask for status_flags */
	uint32_t field_flags;				/*!< Field flags for this message @see ddp_audio_rtp_field_flags */
	uint32_t address_prefix;			/*!< IPv4 Address Prefix -  network byte order */
	uint16_t destination_port;			/*!< RTP flow destination port */
	uint16_t fpp;						/*!< frames per packet; RTP transmit flow packet time */
	uint32_t latency;					/*!< RTP receive flow latency */
};

//! @cond Doxygen_Suppress
typedef struct ddp_audio_rtp_info ddp_audio_rtp_info_t;
//! @endcond

/**@}*/

/**
* \addtogroup AudioFuncs
* @{
*/

/**
* Read data fields from a DDP RTP audio query response message.
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] offset Starting point in the packet buffer to read this message
* @param[out] info Pointer to RTP audio data structure (memory should be allocated by the caller of this function) @see ddp_audio_rtp_info
* @return AUD_SUCCESS if the DDP RTP audio config or query response message was successfully read, else
*	a non AUD_SUCCESS value	is returned
*/
aud_error_t
ddp_read_audio_rtp_query_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_audio_rtp_info_t * out_info
);

/**
* Creates a DDP RTP audio request message to query the device state. This message does not change the state of the device.
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] request_id A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP RTP audio config request message was successfully created, else a non AUD_SUCCESS
*	value is returned
*/
aud_error_t
ddp_add_audio_rtp_query_request
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

#endif //_DDP_RTP_AUDIO_QUERY_H
