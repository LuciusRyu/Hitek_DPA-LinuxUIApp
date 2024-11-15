/*
* File     : routing_client_extension.h
* Created  : July 2020
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Routing extension DDP client helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/
#ifndef _DDP_ROUTING_CLIENT_EXTENSION__H
#define _DDP_ROUTING_CLIENT_EXTENSION__H

#include "packet.h"
#include "shared_structures.h"
#include "routing_manual_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file routing_client_extension.h
* @brief All routing DDP client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
* \addtogroup Routing
* @{
*/

/**
* \addtogroup RoutingEnums
* @{
*/

/**
* @enum ddp_flow_state_flags
* @brief Tx or Rx flow state information has more data.
*/
enum
{
	DDP_FLOW_STATE_FLAG__RTP_AUDIO_DATA = 1 << 0,		/*!< Flow state has RTP audio data */
};

/**@}*/

/**
* \addtogroup RoutingStructs
* @{
*/

/**
* @struct ddp_rx_flow_params_with_extension
* @brief Structure used to retreive the Rx flow state related parameters with extension
*/
struct ddp_rx_flow_params_with_extension
{
	uint16_t flow_id;					//!< Non-zero ID of this flow
	uint16_t num_slots;					//!< number of Rx slots associated with this flow
	uint16_t num_addresses;				//!< number of IP address and port pairs associated with this flow
	char * flow_label;					//!< Null-terminated flow label string, the caller of the function does not need to allocate memory for this field
	uint16_t status;					//!< Current status of the flow @see ddp_routing_flow_status
	uint16_t flow_flags;				//!< Flow flags for Rx @see ddp_routing_rx_flow_flags or Tx @see ddp_routing_tx_flow_flags
	uint16_t flow_flags_mask;			//!< Bitwise OR'd mask which specify which channel_flags are valid
	uint32_t user_conf_flags;			//!< Bitwise OR'd user flow flags @see ddp_routing_flow_user_flags
	uint32_t user_conf_mask;			//!< Bitwise OR'd mask to indicate which of the flow flags are valid
	uint8_t transport;					//!< Audio transport protocol
	uint8_t avail_mask;					//!< Bitmask of configured interfaces (see also addresses)
	uint8_t active_mask;				//!< Bitmask of active interfaces
	uint32_t sample_rate;				//!< Audio sample rate used for this flow
	uint32_t latency;					//!< Audio latency in microseconds used for this flow
	uint16_t encoding;					//!< Audio encoding used for this flows @see audio_supported_encoding
	uint16_t fpp;						//!< Frames per packet used for this flow
	uint8_t flow_state_flags;			//!< @see ddp_flow_state_flags. Flow state has more data.
};

/**
* @brief Structure used to retreive the Tx flow state related parameters with extension
*/
struct ddp_tx_flow_params_with_extension
{
	uint16_t flow_id;					//!< Non-zero ID of this flow
	uint16_t num_slots;					//!< number of Tx slots associated with this flow
	uint16_t num_addresses;				//!< number of IP address and port pairs associated with this flow
	char * flow_label;					//!< Offset from the start of the TLV0 header to the start of the null-terminated flow label string
	uint16_t status;					//!< Current status of the flow @see ddp_routing_flow_status
	uint16_t flow_flags;				//!< Flow flags for Rx @see ddp_routing_rx_flow_flags or Tx @see ddp_routing_tx_flow_flags
	uint16_t flow_flags_mask;			//!< Bitwise OR'd mask which specify which channel_flags are valid
	uint32_t user_conf_flags;			//!< Bitwise OR'd user flow flags @see ddp_routing_flow_user_flags
	uint32_t user_conf_mask;			//!< Bitwise OR'd mask to indicate which of the flow flags are valid
	uint16_t peer_flow_id;				//!< Flow ID of the peer, e.g. if this is a Rx flow, then this is the Tx flow ID
	char * peer_flow_label;				//!< Offset from the start of the TLV0 header to the start of the null-terminated peer flow label string
	uint32_t peer_device_id[2];			//!< 8-byte Dante device ID
	uint16_t peer_process_id;			//!< Process ID of the peer device
	char * peer_device_label;			//!< Offset from the start of the TLV0 header to the start of the null-terminated peer device label string
	uint8_t transport;					//!< Audio transport protocol, currently only DDP_AUDIO_TRANSPORT_ATP is supported
	uint8_t avail_mask;					//!< Bitmask of configured interfaces (see also addresses)
	uint8_t active_mask;				//!< Bitmask of active interfaces
	uint32_t sample_rate;				//!< Audio sample rate used for this flow
	uint32_t latency;					//!< Audio latency in microseconds used for this flow
	uint16_t encoding;					//!< Audio encoding used for this flows @see audio_supported_encoding
	uint16_t fpp;						//!< Frames per packet used for this flow
	uint8_t flow_state_flags;			//!< @see ddp_flow_state_flags. Flow state has more data.
};

/**
* @struct ddp_rtp_audio_flow_params
* @brief Structure used to configure or retreive the RTP Audio specific flow parameters
*/
struct ddp_rtp_audio_flow_params
{
	uint32_t field_flags;				//!< Flags showing params that have valid values.
	uint32_t sdp_session_version_hi;	//!< Most significant 32-bit SDP session version. Ignored for request
	uint32_t sdp_session_version_lo;	//!< Least significant 32-bit SDP session version. Ignored for request
	uint32_t sdp_session_id_hi;			//!< Most significant 32-bit SDP session Identity
	uint32_t sdp_session_id_lo;			//!< Least significant 32-bit SDP session Identity
	uint32_t clock_offset;				//!< media clock offset of transmitter
	ddp_ip_addr_nw_t source_address;	//!< IPv4 address of transmitter
};

/** @cond */
typedef struct ddp_rx_flow_params_with_extension ddp_rx_flow_params_with_extension_t;
typedef struct ddp_tx_flow_params_with_extension ddp_tx_flow_params_with_extension_t;
typedef struct ddp_rtp_audio_flow_params ddp_rtp_audio_flow_params_t;
/** @endcond */

/**@}*/

/**
* \defgroup RoutingFuncs DDP Routing Functions
* @{
*/

/**
* Read data fields from a DDP routing Rx flow configuration state response message for a particular flow.
*
* @note This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function
*
* @param[in] message_info Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter
*	after calling the ddp_read_routing_rx_flow_config_state_response_header
* @param[in] flow_idx Index into the collection of flow config state data blocks in this response message. Values for this parameter
*	range from 0 to number of flows - 1.
* @param[out] out_rx_flow Pointer to a Rx flow state information structure (memory should be allocated by the caller of this function)
*	@see ddp_rx_flow_params_with_extension_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value
*	is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_params_with_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_rx_flow_params_with_extension_t * out_rx_flow
);

/**
* Reads data fields from a DDP routing Tx flow configuration response message for a particular flow.
*
* @note This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function
*
* @param[in] message_info Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter
*	after calling the ddp_read_routing_tx_flow_config_state_response_header function
* @param[in] flow_idx Index into the collection of flow config state data blocks in this response message. Values for this parameter
*	range from 0 to number of flows - 1.
* @param[out] out_tx_flow Pointer to a Tx flow state information structure (memory should be allocated by the caller of this function)
*	@see ddp_tx_flow_state_params_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_params_with_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_tx_flow_params_with_extension_t * out_tx_flow
);

/**
* Reads the RTP audio flow data from a DDP routing flow configuration state response message for a particular Tx or Rx flow.
*
* @note This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_flow_params_with_extension() or
* ddp_read_routing_tx_flow_config_state_response_flow_params_with_extension().
*
* @note The user should confirm that flow_state_flags field in ddp_rx_flow_params_with_extension_t or ddp_tx_flow_params_with_extension_t has
* DDP_FLOW_STATE_FLAG__RTP_AUDIO_DATA flag set.
*
* @param[in] message_info Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after
*	calling the ddp_read_routing_rx_flow_config_state_response_header
* @param[in] flow_idx Index into the collection of flow config state data blocks in this response message. Values for this parameter range
*	from 0 to number of flows - 1.
* @param[out] out_rtp_params Pointer to the RTP data of flow state information structure (memory should be allocated by the caller of this
*	function) @see ddp_rtp_audio_flow_params_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the RTP extension of DDP routing Rx flow configuration state response message was successfully read, else a
*	non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_flow_config_state_response_rtp_extension
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	ddp_rtp_audio_flow_params_t * out_rtp_params
);

/**
* Creates a DDP routing RTP Tx flow configuration request message. This message can only be used to change the state of the device.
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] request_id A non-zero sequence number which is populated in this message
* @param[in] manual_tx_flow Pointer to a structure which has parameters related to how to create the manual Tx flow @see
*	ddp_manual_tx_flow_params_t
* @param[in] rtp_params Only meaningful field of this struct for the current fnction is session id (which is optional)
* @return AUD_SUCCESS if the DDP routing manual Tx flow configuration request message was successfully created, else a
*	non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_rtp_tx_flow_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_tx_flow_params_t * manual_tx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
);

/**
* Creates a DDP routing RTP Rx flow manual configuration request message. This message can only be used to change the state of the device.
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] request_id A non-zero sequence number which is populated in this message
* @param[in] manual_rx_flow Pointer to a structure which has parameters related to how to create the manual RTP Rx flow @see
*	ddp_manual_rx_flow_params_t
* @param[in] rtp_params Only meaningful field of this struct for the current function is media clock_offset (which is optional)
* @return AUD_SUCCESS if the DDP routing manual Rx flow configuration request message was successfully created, else a
*	non AUD_SUCCESS value is returned
*/

aud_error_t
ddp_add_routing_rtp_rx_flow_manual_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
);

/**
* Creates a DDP routing RTP Rx flow configuration with SDP data. This message can only be used to change the state of the device.
* @param[in] packet_info Pointer to a structure which has information about the DDP packet buffer
* @param[in] request_id A non-zero sequence number which is populated in this message
* @param[in] manual_rx_flow Pointer to a structure which has parameters related to how to create the manual RTP Rx flow @see
*	ddp_manual_rx_flow_params_t
* @param[in] rtp_params Pointer to a structure which has parameters related to RTP data.
* @return AUD_SUCCESS if the DDP routing manual Rx flow configuration request message was successfully created, else a
*	non AUD_SUCCESS value is returned
*/

aud_error_t
ddp_add_routing_rtp_rx_flow_sdp_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manual_rx_flow_params_t * manual_rx_flow,
	ddp_rtp_audio_flow_params_t * rtp_params
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif //_DDP_ROUTING_CLIENT_EXTENSION__H
