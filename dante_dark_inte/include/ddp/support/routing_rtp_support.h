/*
* File     : routing_rtp_support.h
* Created  : September 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : RTP routing extension utility definitions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_ROUTING_RTP_SUPPORT_H
#define _DDP_ROUTING_RTP_SUPPORT_H


//----------
#include "../packet.h"
#include "../proto/routing_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ddp_util_aes67_flow_info
{
	uint32_t field_flags;
	ddp_ip_addr_nw_t sap_origin_addr;
	uint64_t sdp_session_id;
	uint64_t sdp_session_version;
	uint32_t media_clock_offset;
} ddp_util_aes67_flow_info_t;


// read rtp info from raw flow data
aud_error_t
ddp_util_read_rtp_flow_data_extension
(
	const ddp_message_read_info_t * mri,
	const ddp_routing_flow_data_t * fd,
	ddp_util_aes67_flow_info_t * info
);

// add rtp info to raw flow data
aud_error_t
ddp_util_add_rtp_flow_data_extension
(
	ddp_message_write_info_t * mwi,
	ddp_routing_flow_data_t * txf,
	const ddp_util_aes67_flow_info_t * info
);


#ifdef __cplusplus
}
#endif

#endif
