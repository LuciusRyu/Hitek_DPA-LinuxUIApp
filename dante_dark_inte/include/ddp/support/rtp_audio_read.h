/*
* File     : rtp_audio_read.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Support functions to read DDP RTP Audio specific config.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/
#ifndef _DDP_RTP_AUDIO_READ_H
#define _DDP_RTP_AUDIO_READ_H


//----------
// Include

#include "../rtp_audio_query.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
* @file rtp_audio_read.h
* @brief RTP audio DDP helper functions.
* NOTE: These functions are not for public use.
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
* \addtogroup AudioFuncs
* @{
*/

/**
* Reads the payload out from a DDP audio RTP query message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param info [in] Pointer to a structure which has information about the DDP packet buffer @see ddp_audio_rtp_info
* @return AUD_SUCCESS if the DDP audio basic request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_audio_rtp_config_read_payload
(
	const ddp_message_read_info_t * message_info,
	ddp_audio_rtp_info_t *info
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif //_DDP_RTP_AUDIO_READ_H
