/*
* File     : clock_v2_read.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Helper functions to read DDP Clock v2 config
*
* This software is Copyright (c) 2004-2020, Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_CLOCK_V2_READ__H
#define _DDP_CLOCK_V2_READ__H

//----------
// Include

#include "../clock_v2_query.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
* @file clock_v2_read.h
* @brief DDP Clock v2 helper functions.
* NOTE: These functions are not for public use.
*/

/**
* \defgroup DDP DDP
* @{
*/

/**
* \defgroup DDP Clock v2 Structures for client
* @{
*/


// Reading

aud_error_t
ddp_clock_v2_config_read_payload
(
	const ddp_message_read_info_t * message_info,
	ddp_clock_v2_info_t *info
);


#ifdef __cplusplus
}
#endif

#endif //

