/*
* File     : ddp.h
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : Top level DDP definitions
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_H
#define _DDP_H

#include <dante_common.h>

// DDP versions are uint 16 in 4.4.8 format
enum
{
	DDP_VERSION_1_0_0		= 0x1000,
	DDP_VERSION_CURRENT		= DDP_VERSION_1_0_0
};
typedef uint16_t ddp_version_t;

#define DDP_VERSION_MAJOR(V) (((V) & 0xF000)  >> 12)
#define DDP_VERSION_MINOR(V) (((V) & 0x0F00)  >> 8)
#define DDP_VERSION_BUGFIX(V) (((V) & 0x000F) >> 0)

// offsets with in a DDP packet in host order
typedef uint16_t ddp_offset_t;

// offsets with in a DDP packet in network order
typedef uint16_t ddp_raw_offset_t;

// DDP request ids are a uint16_t, where 0 represents an event
typedef uint16_t ddp_request_id_t;

// DDP opcodes are a uint16_t, however the MSB is reserved.
typedef uint16_t ddp_opcode_t;

// DDP status codes are a uint16_t
enum ddp_status
{
	DDP_STATUS_NOERROR						= 0,	//[0] no error
	DDP_STATUS_ERROR_OTHER					= 1,	//[1] error - other (undefined error)
	DDP_STATUS_ERROR_UNSUPPORTED_OPCODE		= 2,	//[2] error - unsupported opcode
	DDP_STATUS_ERROR_UNSUPPORTED_VERSION	= 3,	//[3] error - unsupported DDP version
	DDP_STATUS_ERROR_INVALID_FORMAT			= 4,	//[4] error - invalid packet/message format
	DDP_STATUS_ERROR_INVALID_DATA			= 5,	//[5] error - invalid data
	DDP_STATUS_ERROR_PERMISSION_DENIED		= 6,	//[6] error - operation is not allowed / permitted
	DDP_STATUS_ERROR_REBOOT_REQUIRED		= 7,	//[7] error - operation cannot be performed because device is in read only mode and requires a reboot
	DDP_STATUS_ERROR_NOT_READY				= 8,	//[8] error - message cannot be handled because device is not ready to handle it, e.g. device is in the middle of an upgrade
	//OTHER RESERVED
};
typedef uint16_t ddp_status_t;

/**
* @struct ddp_event_timestamp
* @brief Structure format for a the DDP event
*/
typedef struct ddp_event_timestamp {
	uint32_t seconds;				/*!< seconds */
	uint32_t subseconds;			/*!< subseconds (no defined unit) */
} ddp_event_timestamp_t;

/**
* @union ddp_block_header
* @brief Structure format for a the DDP block header
*/
typedef union ddp_block_header
{
	struct
	{
		uint16_t length_bytes; 	/*!< the total length of this block */
		uint16_t opcode;		/*!< opcode for this block */
	} _;						/*!< ddp_block_header structure definition */

	uint32_t __alignment;		/*!< enforce 32-bit alignment for all packet blocks */
} ddp_block_header_t;


/**
* @struct ddp_packet_header
* @brief Structure format for a the DDP packet header, must be the first block in a packet
*/
typedef struct ddp_packet_header
{
	ddp_block_header_t block;		/*!< The block header */
	uint16_t version;				/*!< version */

	union {
		uint16_t padding0;				/*!< if no extension header set padding0 to 0x0000 */

		struct {
			uint8_t flags;				/*!< flags */
			uint8_t extension_offset;	/*!< offset to the extension */
		} ext;							/*!< DDP packet header extension */
	} _; /*!< ddp_packet_header extension definition */
} ddp_packet_header_t;

enum
{
	// Bits 0-1 of flag field indicate 'direction' of packet.

		// Useful for bi-directional pipes where each end of the pipe sometimes
		// functions as a client (issuing requests) and sometimes as a server
		// (sending responses and events)

		// NOTE: request and response messages MUST NOT be put in the same packet

	DDP_PACKET_HEADER_FLAG_DIRECTION_MASK = 0x3,
	DDP_PACKET_HEADER_FLAG_DIRECTION_NONE = 0x0,
		// Implicit or undefined direction
	DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST = 0x2,
		// Packet is a request (client -> server)
	DDP_PACKET_HEADER_FLAG_DIRECTION_RESPONSE = 0x3,
		// Packet is a response or event (server -> client)

	DDP_PACKET_HEADER_FLAG_LINK_INFRASTRUCTURE = 0x80,
		// Packet is for link endpoint and not client code
};


/**
* @struct ddp_message_header
* @brief Structure format for a DDP message
*/
typedef struct ddp_message_header
{
	ddp_block_header_t block;			/*!< The block header */
	uint16_t subheader_length_bytes;	/*!< Length of the subheader in bytes */
	uint16_t payload_length_bytes;		/*!< Length of the payload in bytes */

	// message header is followed by subheader and then by payload
} ddp_message_header_t;

/**
* @struct ddp_request_subheader
* @brief Structure format for the DDP request subheader
*/
typedef struct ddp_request_subheader
{
	ddp_request_id_t request_id;	/*!< request id for this message, must be non-zero */
	uint16_t padding;				/*!< pad */
} ddp_request_subheader_t;

#define DDP_REQUEST_SUBHEADER_MIN_SIZE 4 // subheader might grow one day (unlikely but posible) but will never shrink

/**
* @struct ddp_response_subheader
* @brief Structure format for the DDP response subheader
*/
typedef struct ddp_response_subheader
{
	ddp_request_id_t request_id;	/*!< request id for that is response is for, 0 if an event */
	ddp_status_t status;			/*!< status @see ddp_status */
} ddp_response_subheader_t;

/**
* @struct ddp_local_subheader
* @brief Structure format for the DDP local subheader
*/
typedef struct ddp_local_subheader
{
	ddp_event_timestamp_t timestamp;	/*!< timestamp */
} ddp_local_subheader_t;

/**
* @struct ddp_monitoring_subheader
* @brief Structure format for the DDP monitoring subheader
*/
typedef struct ddp_monitoring_subheader
{
	uint16_t monitor_seqnum;		/*!< monitoring sequence number */
	uint16_t padding;				/*!< pad */
} ddp_monitoring_subheader_t;

#endif // _DDP_H
