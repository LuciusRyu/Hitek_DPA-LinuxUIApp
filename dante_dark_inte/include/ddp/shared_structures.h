/*
 * File     : ddp_shared_structures.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_SHARED_STRUCTURES__H
#define _DDP_SHARED_STRUCTURES__H

//----------
// Include

#include "ddp.h"

//----------
// Audio Transports

/**
* @enum ddp_audio_transport
* @brief Audio Transport protocol values used for various messages
*/
enum ddp_audio_transport
{
	DDP_AUDIO_TRANSPORT_UNDEF	= 0,		/*!< Undefined audio transport */
	DDP_AUDIO_TRANSPORT_ATP	= 1,			/*!< Dante ATP */
	DDP_AUDIO_TRANSPORT_RTP = 2			/*!< Audio RTP */
};


//----------
// Addresses

/**
* @enum ddp_address_family
* @brief Address family values used for various messages
*/
enum ddp_address_family
{
	DDP_ADDRESS_FAMILY_UNDEF = 0, 	/*!< Undefined address family */
	DDP_ADDRESS_FAMILY_INET	= 2, 	/*!< IPv4 address family */
};

/**
* @union ddp_ip_addr_nw
* @brief IP address representation in lib DDP
*/
union ddp_ip_addr_nw
{
	uint32_t ip_addr;		/*!< IPv4 address in network byte order */
	uint8_t ip_octs[4];		/*!< Array to access individual octets of the IPv4 address */
};

/** @cond */
typedef union ddp_ip_addr_nw ddp_ip_addr_nw_t;
/** @endcond */

/**
* @struct ddp_addr_inet
* @brief Structure format for the DDP address format (IP address : port)
*/
struct ddp_addr_inet
{
	uint16_t family;			/*!< IP address family @see ddp_address_family */
	uint16_t port;				/*!< UDP/IP port */
	ddp_ip_addr_nw_t in_addr;	/*!< IP address */
};

/** @cond */
typedef struct ddp_addr_inet ddp_addr_inet_t;
/** @endcond */

/**
* @struct ddp_addr_inet_nw_ip
* @brief Structure format for the DDP address format (IP address : port)
*/
struct ddp_addr_inet_nw_ip
{
	uint16_t family;			/*!< IP address family @see ddp_address_family */
	uint16_t port;				/*!< UDP/IP port */
	ddp_ip_addr_nw_t in_addr;		/*!< IP address in network byte order */
};

/** @cond */
typedef struct ddp_addr_inet_nw_ip ddp_addr_inet_nw_ip_t;
/** @endcond */

//-----------------
// Identity Ranges

/**
* @struct ddp_id_range
* @brief Structure format for the ID ranges used for "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_range
{
	uint16_t from;				/*!< Starting ID value, this value should be non-zero */
	uint16_t to;				/*!< Ending ID value, this value should be non-zero */
};

/** @cond */
typedef struct ddp_id_range ddp_id_range_t;
/** @endcond */

/**
* @struct ddp_id_ranges
* @brief Structure format for the heap section of the payload of the "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_ranges
{
	uint16_t curr;				/*!< Number of valid elements in the array */
	uint16_t max;				/*!< Maximum number of elements in the array (fixed at creation) */
	ddp_id_range_t range[1];	/*!< Pair of range values @see ddp_id_range */
};

/** @cond */
typedef struct ddp_id_ranges ddp_id_ranges_t;
/** @endcond */

//-----------
// Arrays

/**
* @struct ddp_id_range_payload
* @brief Structure format for the payload of the "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_range_payload
{
	ddp_raw_offset_t ranges_offset;		/*!< Offset to a structure that contains information about the channel ID or flow ID range */
	uint16_t pad0;						/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_id_range_payload ddp_id_range_payload_t;
/** @endcond */

/**
* @struct ddp_offset_array_payload
* @brief Structure format for the payload of the "routing Rx/Tx channel config state response" message
*/
struct ddp_offset_array_payload
{
	ddp_raw_offset_t array_offset;		/*!< Offset from the start of the TLV0 header to the start of the a ddp_array structure @see ddp_array_t */
	uint16_t pad0;						/*!< Pad for alignment - must be '0' */
};

//! @cond Doxygen_Suppress
typedef struct ddp_offset_array_payload ddp_offset_array_payload_t;
//! @endcond Doxygen_Suppress

/**
* @struct ddp_array
* @brief Structure format for part of the heap section of the "routing Rx channel config state response" message
*/
struct ddp_array
{
	/*!< @struct ddp_array_header */
	struct ddp_array_header
	{
		uint16_t curr;				/*!< Number of valid elements in the array */
		uint16_t max;				/*!< Maximum number of elements in the array (fixed at creation) */
	} header;  /*!< header */
	ddp_raw_offset_t offset[1];		/*!< Array of offsets to elements */
};

/** @cond */
typedef struct ddp_array ddp_array_t;
/** @endcond */

/**
* @struct ddp_array_payload
* @brief Structure format of the payload for the "routing Rx or Tx channel request" and "routng subscribe request" messages
*/
struct ddp_array_payload
{
	uint16_t num_elements;			/*!< Number of Rx or Tx channels to modify */
	uint16_t element_size;			/*!< Size of each label structure in bytes @see ddp_name_id_pair_t, this field should be set to 4 bytes */
	uint16_t array_offset;			/*!< Offset from the start of the TLV0 header to the start of the label structure @see ddp_name_id_pair_t */
	uint16_t pad0;					/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_array_payload ddp_array_payload_t;
/** @endcond */

//----------

#endif // _DDP_SHARED_STRUCTURES__H
