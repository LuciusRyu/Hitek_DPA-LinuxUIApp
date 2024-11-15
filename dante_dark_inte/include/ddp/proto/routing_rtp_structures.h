/*
* File     : routing_rtp_structures.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Audio RTP flow DDP messages structure definitions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_ROUTING_RTP_STRUCTURES_H
#define _DDP_ROUTING_RTP_STRUCTURES_H


//----------

#include "../ddp.h"

#include "../shared_structures.h"

/**
* @file routing_rtp_structures.h
* @brief Audio RTP routing DDP messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Routing
*  @{
*/

/**
* \addtogroup RoutingEnums
* @{
*/

/**
* @enum ddp_rtp_flow_fields
* @brief field flags for ddp_raw_rtp_flow_extension_data
*/
enum {
	RTP_FLOW_DATA_FIELD_FLAG__SESSION_VERSION = 1 << 0,		//!< SDP Session version field is valid
	RTP_FLOW_DATA_FIELD_FLAG__SESSION_ID = 1 << 1,			//!< SDP Session Identity field is valid
	RTP_FLOW_DATA_FIELD_FLAG__CLOCK_OFFSET = 1 << 2,		//!< SDP Clock Offset field is valid
	RTP_FLOW_DATA_FIELD_FLAG__SOURCE_ADDRESS = 1 << 3		//!< SDP Source address field is valid
};

/**@}*/

/**
* \addtogroup RoutingStructs
* @{
*/

/**
* @struct ddp_raw_rtp_flow_extension_data
* @brief Structure format for the RTP flow data. This forms part of the 'transport_extension_offset' of 'ddp_routing_flow_data' message.
*/
struct ddp_raw_rtp_flow_extension_data
{
	uint32_t field_flags;				//!< Field flags for the RTP flow data. @see ddp_rtp_flow_fields for existing fields.
	uint32_t sdp_session_ver_hi;		//!< Most significant 32 bits of SDP session version
	uint32_t sdp_session_ver_lo;		//!< Least significant 32 bits of SDP session version
	uint32_t sdp_session_id_hi;			//!< Most significant 32 bits of SDP session Identity
	uint32_t sdp_session_id_lo;			//!< Least significant 32 bits of SDP session Identity
	uint32_t clock_offset;				//!< RTP flow Clock Offset
	ddp_ip_addr_nw_t source_address;	//!< RTP flow's source IPv4 address
};

//! @cond Doxygen_Suppress
typedef struct ddp_raw_rtp_flow_extension_data ddp_raw_rtp_flow_extension_data_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif	//_DDP_ROUTING_RTP_STRUCTURES_H
