/*
* File     : ddp_clocking_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All clocking DDP client read helper functions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/clocking_client.h"
#include "../include/ddp/proto/clocking_structures.h"

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_clock_basic_legacy_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t* out_capability_flags,
	uint8_t* out_clock_state,
	uint8_t* out_mute_state,
	uint8_t* out_ext_wc_state,
	uint8_t* out_preferred,
	uint32_t* out_drift
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_clock_basic_legacy_response_t * header;
	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_BASIC_LEGACY;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_BASIC_LEGACY | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(&message_info, expected_opcode, sizeof(ddp_clock_basic_legacy_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_basic_legacy_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if(out_clock_state)
	{
		*out_clock_state = header->payload.clock_state_legacy;
	}
	if(out_mute_state)
	{
		*out_mute_state = header->payload.mute_state;
	}
	if(out_ext_wc_state)
	{
		*out_ext_wc_state = header->payload.ext_wc_state;
	}
	if(out_preferred)
	{
		*out_preferred = header->payload.preferred;
	}
	if(out_drift)
	{
		*out_drift = ntohl(header->payload.drift);
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_clock_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_clock_config_control_params_t * out_config_params,
	uint16_t * out_num_clock_ports
)
{

	aud_error_t result;
	ddp_clock_config_response_t * header;
	uint16_t valid_flags;
	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_CONFIG;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_CONFIG | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(out_message_info, expected_opcode, sizeof(ddp_clock_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_config_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	valid_flags = ntohs(header->payload.valid_flags);
	if (out_config_params)
	{
		if (valid_flags & CLOCK_CONFIG_RSP_PREFERRED_VALID)
		{
			out_config_params->preferred = header->payload.preferred;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_WC_SYNC_VALID)
		{
			out_config_params->ext_word_clock_sync = header->payload.word_clock_sync;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_LOGGING_VALID)
		{
			out_config_params->logging = header->payload.logging;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_MULTICAST_VALID)
		{
			out_config_params->multicast = header->payload.multicast;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_SLAVE_ONLY_VALID)
		{
			out_config_params->slave_only = header->payload.slave_only;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_CLOCK_PROTOCOL_VALID)
		{
			out_config_params->clock_protocol = header->payload.clock_protocol;
			out_config_params->clock_protocol_mask = header->payload.clock_protocol_mask;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_UNICAST_DELAY_CLOCK_PROTOCOL_VALID)
		{
			out_config_params->unicast_delay_clock_protocol = header->payload.unicast_delay_clock_protocol;
			out_config_params->unicast_delay_clock_protocol_mask = header->payload.unicast_delay_clock_protocol_mask;
		}
	}

	if (out_num_clock_ports)
	{
		*out_num_clock_ports = ntohs(header->payload.num_clock_port_structures);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_config_response_port
(
	const ddp_message_read_info_t * message_info,
	uint16_t port_idx,
	ddp_clock_config_port_params_t * out_port_params
)
{
	ddp_heap_read_info_t heap_info;
	aud_error_t result;
	ddp_clock_config_response_t * header;
	ddp_clock_config_port_t * clock_port;

	header = (ddp_clock_config_response_t *)message_info->header.header;

	if (port_idx >= ntohs(header->payload.num_clock_port_structures))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct
				(
					message_info,
					&heap_info,
					ntohs(header->payload.clock_port_structs_offset),
					ntohs(header->payload.num_clock_port_structures),
					ntohs(header->payload.clock_port_structure_size)
				);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	clock_port = ((ddp_clock_config_port_t *)heap_info._.array32) + port_idx;

	out_port_params->port_id = ntohs(clock_port->port_id);
	out_port_params->enable_port = clock_port->enable_port;

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_pullup_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_current_pullup,
	uint8_t * out_reboot_pullup,
	const uint8_t ** out_current_subdomain,
	uint16_t * out_current_subdomain_length,
	const uint8_t ** out_reboot_subdomain,
	uint16_t* out_reboot_subdomain_length,
	const clock_supported_pullup_t ** out_supported_pullups,
	uint16_t * out_num_supported_pullups
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_clock_pullup_response_t * header;
	uint16_t valid_flags;
	ddp_heap_read_info_t heap_info;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_CLOCK_PULLUP, sizeof(ddp_clock_pullup_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_pullup_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	//get the valid flags
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_current_pullup && (valid_flags & CLOCK_PULLUP_CURR_PULLUP_VALID) )
	{
		*out_current_pullup = header->payload.current_pullup;
	}

	if (out_reboot_pullup && (valid_flags & CLOCK_PULLUP_REBOOT_PULLUP_VALID) )
	{
		*out_reboot_pullup = header->payload.reboot_pullup;
	}

	if (out_current_subdomain && out_current_subdomain_length)
	{
		const uint8_t * current_subdomain = NULL;
		uint16_t current_subdomain_length = 0;

		if (valid_flags & CLOCK_PULLUP_CURR_SUBDOMAIN_VALID)
		{
			if
			(
				ddp_read_heap_array8
				(
					&message_info,
					&heap_info,
					ntohs(header->payload.current_subdomain_offset),
					ntohs(header->payload.current_subdomain_size)
				) == AUD_SUCCESS
			)
			{
				current_subdomain = heap_info._.array8;
				current_subdomain_length = heap_info.length_bytes;
			}
		}

		*out_current_subdomain = current_subdomain;
		*out_current_subdomain_length = current_subdomain_length;
	}

	if (out_reboot_subdomain && out_reboot_subdomain_length)
	{
		const uint8_t * reboot_subdomain = NULL;
		uint16_t reboot_subdomain_length = 0;
		if (valid_flags & CLOCK_PULLUP_REBOOT_SUBDOMAIN_VALID)
		{
			if
			(
				ddp_read_heap_array8
				(
					&message_info,
					&heap_info,
					ntohs(header->payload.reboot_subdomain_offset),
					ntohs(header->payload.reboot_subdomain_size)
				) == AUD_SUCCESS
			)
			{
				reboot_subdomain = heap_info._.array8;
				reboot_subdomain_length = heap_info.length_bytes;
			}
		}

		*out_reboot_subdomain = reboot_subdomain;
		*out_reboot_subdomain_length = reboot_subdomain_length;
	}

	if (out_supported_pullups && out_num_supported_pullups)
	{

		clock_supported_pullup_t * supported_pullups = NULL;
		uint16_t num_supported_pullups = 0;

		if (valid_flags & CLOCK_PULLUP_SUPPORTED_PULLUPS_VALID)
		{
			//check that the sizes match
			if(sizeof(clock_supported_pullup_t) != ntohs(header->payload.supported_pullups_size))
			{
				return AUD_ERR_INVALIDDATA;
			}

			if
			(
				ddp_read_heap_arraystruct
				(
					&message_info,
					&heap_info,
					ntohs(header->payload.supported_pullups_offset),
					ntohs(header->payload.num_supported_pullups),
					ntohs(header->payload.supported_pullups_size)
				) == AUD_SUCCESS
			)
			{
				*out_supported_pullups = (clock_supported_pullup_t*)heap_info._.array16;
				*out_num_supported_pullups = heap_info.num_elements;
			}
		}

		*out_supported_pullups = supported_pullups;
		*out_num_supported_pullups = num_supported_pullups;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t * out_capability_flags,
	uint8_t * out_clock_source,
	uint8_t * out_clock_state,
	uint8_t * out_servo_state,
	uint8_t * out_preferred,
	uint8_t * out_mute_state,
	uint8_t * out_ext_wc_state,
	uint8_t * out_clock_stratum,
	int32_t * out_drift,
	int32_t * out_max_drift,
	uint8_t const ** out_uuid,
	uint8_t const ** out_master_uuid,
	uint8_t const ** out_grandmaster_uuid,
	uint8_t * out_num_ports,
	uint8_t * out_port_size
)
{
	aud_error_t result;
	ddp_clock_basic2_response_t * header;
	ddp_heap_read_info_t heap_info;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_BASIC2;
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_BASIC2 | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(out_message_info, expected_opcode, sizeof(ddp_clock_basic2_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_basic2_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if (out_clock_source)
	{
		*out_clock_source = header->payload.clock_source;
	}
	if (out_clock_state)
	{
		*out_clock_state = header->payload.clock_state;
	}
	if (out_servo_state)
	{
		*out_servo_state = header->payload.servo_state;
	}
	if (out_preferred)
	{
		*out_preferred = header->payload.preferred;
	}
	if (out_mute_state)
	{
		*out_mute_state = header->payload.mute_state;
	}
	if (out_ext_wc_state)
	{
		*out_ext_wc_state = header->payload.ext_wc_state;
	}
	if (out_clock_stratum)
	{
		*out_clock_stratum = header->payload.clock_stratum;
	}
	if (out_drift)
	{
		*out_drift = ntohl(header->payload.drift);
	}
	if (out_max_drift)
	{
		*out_max_drift = ntohl(header->payload.max_drift);
	}
	if (out_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.uuid_offset),
				header->payload.uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_uuid = heap_info._.array8;
		}
		else
		{
			*out_uuid = NULL;
		}
	}
	if (out_master_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.master_uuid_offset),
				header->payload.master_uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_master_uuid = heap_info._.array8;
			
		}
		else
		{
			*out_master_uuid = NULL;
		}
	}
	if (out_grandmaster_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.grandmaster_uuid_offset),
				header->payload.grandmaster_uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_grandmaster_uuid = heap_info._.array8;
		}
		else
		{
			*out_grandmaster_uuid = NULL;
		}
	}
	if (out_num_ports)
	{
		*out_num_ports = header->payload.num_ports;
	}
	if (out_port_size)
	{
		*out_port_size = header->payload.port_size;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_port
(
	const ddp_message_read_info_t * message_info,
	uint8_t port_idx,
	ddp_clock_port_t ** out_port
)
{
	aud_error_t result;
	ddp_clock_basic2_response_t * header;
	ddp_heap_read_info_t heap_info;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (port_idx >= header->payload.num_ports)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
		&heap_info,
		ntohs(header->payload.port_array_offset),
		header->payload.num_ports,
		header->payload.port_size);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	*out_port = ((ddp_clock_port_t *)heap_info._.array32) + port_idx;

	(*out_port)->flags = ntohs((*out_port)->flags);
	(*out_port)->port_id = ntohs((*out_port)->port_id);

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_port_v2
(
	const ddp_message_read_info_t * message_info,
	uint8_t port_idx,
	ddp_clock_port_read_info_t * out_port_info
)
{
	aud_error_t result;
	ddp_clock_basic2_response_t * header;
	ddp_heap_read_info_t heap_info;
	ddp_clock_port_raw_t * raw_port;
	ddp_clock_port_t * legacy_port;
	uint16_t valid_flags;
	if (!message_info || !out_port_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (port_idx >= header->payload.num_ports)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	valid_flags = ntohs(header->payload.valid_flags);

	out_port_info->valid_flags = 0;

	if (!AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_BASIC2_VALID_FLAG__PORT_RAW_ARRAY))
	{
		//old server/firmware - read port data from legacy array

		result = ddp_read_heap_arraystruct(message_info,
			&heap_info,
			ntohs(header->payload.port_array_offset),
			header->payload.num_ports,
			header->payload.port_size);

		if (result != AUD_SUCCESS)
		{
			return result;
		}
		legacy_port = ((ddp_clock_port_t *)heap_info._.array32) + port_idx;

		out_port_info->flags = ntohs(legacy_port->flags);
		out_port_info->port_id = ntohs(legacy_port->port_id);
		out_port_info->castness = legacy_port->castness;
		out_port_info->interface_index = legacy_port->interface_index;
		out_port_info->port_protocol = legacy_port->port_protocol;
		out_port_info->port_state = legacy_port->port_state;
		AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__FLAGS
			| DDP_CLOCK_PORT_VALID_FLAG__PORT_ID | DDP_CLOCK_PORT_VALID_FLAG__CASTNESS
			| DDP_CLOCK_PORT_VALID_FLAG__INTERFACE_INDEX | DDP_CLOCK_PORT_VALID_FLAG__PROTOCOL
			| DDP_CLOCK_PORT_VALID_FLAG__STATE);
		return AUD_SUCCESS;
	}
	else
	{
		result = ddp_read_heap_arraystruct(message_info,
			&heap_info,
			ntohs(header->payload.port_raw_array_offset),
			header->payload.num_ports,
			ntohs(header->payload.port_raw_size));

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		raw_port = ((ddp_clock_port_raw_t *)heap_info._.array32) + port_idx;

		valid_flags = ntohs(raw_port->valid_flags);
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__FLAGS))
		{
			out_port_info->flags = ntohs(raw_port->flags);
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__FLAGS);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__PORT_ID))
		{
			out_port_info->port_id = ntohs(raw_port->port_id);
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__PORT_ID);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__CASTNESS))
		{
			out_port_info->castness = raw_port->castness;
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__CASTNESS);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__INTERFACE_INDEX))
		{
			out_port_info->interface_index = raw_port->interface_index;
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__INTERFACE_INDEX);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__PROTOCOL))
		{
			out_port_info->port_protocol = raw_port->port_protocol;
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__PROTOCOL);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__STATE))
		{
			out_port_info->port_state = raw_port->port_state;
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__STATE);
		}
		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_PORT_RAW_VALID_FLAG__NETWORK_INDEX))
		{
			out_port_info->network_index = ntohs(raw_port->network_index);
			AUD_FLAG_SET(out_port_info->valid_flags, DDP_CLOCK_PORT_VALID_FLAG__NETWORK_INDEX);
		}

	}

	return AUD_SUCCESS;
}

/**
* Interprets the ptp v1 eui48 id from a uuid buffer
* @param uuid [in] Pointer to a uuid buffer in the packet
* @param uuid_size [in] size of to a uuid buffer in the packet
*/
static aud_error_t
ddp_clock_basic2_response_read_uuid_eui48(const uint8_t * uuid, uint8_t uuid_size, ddp_clock_uuid_t* uuid_out) {

	assert(uuid_size >= DDP_CLOCK_UUID_LENGTH);

	// First 6 bytes are the eui48
	memcpy(&uuid_out->data[0], &uuid[0], 6);

	return AUD_SUCCESS;
}

/**
* Interprets the ptp v2 eui64 id from a uuid buffer
* @param uuid [in] Pointer to a uuid buffer in the packet
* @param uuid_size [in] size of to a uuid buffer in the packet
*/
static aud_error_t
ddp_clock_basic2_response_read_uuid_eui64(const uint8_t * uuid, uint8_t uuid_size, ddp_v2_clock_uuid_t* uuid_out) {

	assert(uuid_size >= DDP_CLOCK_UUID_LENGTH);
	
	// Converting 8 byte packet format (6 eui48 bytes followed by extra 2 eui64 bytes to an eui64
	// eg 11:22:33:66:77:88:44:55 becomes 11:22:33:44:55:66:77:88 to 

	// First 3 are in common
	memcpy(&uuid_out->data[0], &uuid[0], 3);
	// last three are bytes 4,5, and 6 in the packet
	memcpy(&uuid_out->data[5], &uuid[3], 3);

	if (uuid_size < DDP_V2_CLOCK_UUID_LENGTH) {

		// Misisng middle two bytes - convert from v1 uuid using standard eui64 conversion
		uuid_out->data[3] = 0xff;
		uuid_out->data[4] = 0xfe;

	}
	else {
		// The middle two bytes are bytes 7 and 8 in the packet
		memcpy(&uuid_out->data[3], &uuid[6], 2);
	}
	
	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_v2
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_clock_basic2_response_read_info_t * out_info
)
{
	ddp_clock_basic2_response_t * header;
	uint16_t valid_flags = 0;
	aud_error_t result = AUD_SUCCESS;

	if (out_info == NULL)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_clock_basic2_response_header(
			packet_info,
			offset,
			out_message_info,
			out_request_id,
			out_status,
			out_info ? &out_info->capability_flags : NULL,
			out_info ? &out_info->clock_source : NULL,
			out_info ? &out_info->clock_state : NULL,
			out_info ? &out_info->servo_state : NULL,
			out_info ? &out_info->preferred : NULL,
			out_info ? &out_info->mute_state : NULL,
			out_info ? &out_info->ext_wc_state : NULL,
			out_info ? &out_info->clock_stratum : NULL,
			out_info ? &out_info->drift : NULL,
			out_info ? &out_info->max_drift : NULL,
			NULL,
			NULL,
			NULL,
			out_info ? &out_info->num_ports : NULL,
			out_info ? &out_info->port_size : NULL
		);

	if (result != AUD_SUCCESS)
		return result;

	//message already validated in above call
	header = (ddp_clock_basic2_response_t *)out_message_info->header.header;

	valid_flags = ntohs(header->payload.valid_flags);

	if (out_info) {

		if (AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_BASIC2_VALID_FLAG__V2_DOMAIN_NUMBER))
		{
			out_info->v2_domain_number = header->payload.v2_domain_number;
			AUD_FLAG_SET(out_info->valid_flags, CLOCK_BASIC2_READ_VALID_FLAG__V2_DOMAIN_NUMBER);
		}

	}	

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_uuid_v1
(
	const ddp_message_read_info_t * message_info,
	ddp_clock_uuid_t* out_uuid_v1
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *) message_info->header.header;

	if (out_uuid_v1)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.uuid_offset),
				header->payload.uuid_size)
			== AUD_SUCCESS
			)
		{
			return ddp_clock_basic2_response_read_uuid_eui48(heap_info._.array8, header->payload.uuid_size, out_uuid_v1);
		}
		
		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}

aud_error_t
ddp_read_clock_basic2_response_master_uuid_v1
(
	const ddp_message_read_info_t * message_info,
	ddp_clock_uuid_t* out_uuid_v1
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (out_uuid_v1)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.master_uuid_offset),
				header->payload.master_uuid_size)
			== AUD_SUCCESS
			)
		{
			return ddp_clock_basic2_response_read_uuid_eui48(heap_info._.array8, header->payload.master_uuid_size, out_uuid_v1);
		}

		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}

aud_error_t
ddp_read_clock_basic2_response_grandmaster_uuid_v1
(
	const ddp_message_read_info_t * message_info,
	ddp_clock_uuid_t* out_uuid_v1
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (out_uuid_v1)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.grandmaster_uuid_offset),
				header->payload.grandmaster_uuid_size)
			== AUD_SUCCESS
			)
		{
			return ddp_clock_basic2_response_read_uuid_eui48(heap_info._.array8, header->payload.grandmaster_uuid_size, out_uuid_v1);
		}

		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}

aud_error_t
ddp_read_clock_basic2_response_uuid_v2
(
	const ddp_message_read_info_t * message_info,
	ddp_v2_clock_uuid_t* out_uuid_v2
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *)message_info->header.header;	

	if (out_uuid_v2)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.uuid_offset),
				header->payload.uuid_size)
			== AUD_SUCCESS
			)
		{
			uint16_t valid_flags = ntohs(header->payload.valid_flags);
			// Cap the uuid size at 6 if validity flag is not set since older devices sent 6 valid bytes with size of 8
			uint8_t size = AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_BASIC2_VALID_FLAG__V2_UUID_FORMAT) ? header->payload.uuid_size : MIN(DDP_CLOCK_UUID_LENGTH, header->payload.uuid_size);
			return ddp_clock_basic2_response_read_uuid_eui64(heap_info._.array8, size, out_uuid_v2);
		}

		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}

aud_error_t
ddp_read_clock_basic2_response_master_uuid_v2
(
	const ddp_message_read_info_t * message_info,
	ddp_v2_clock_uuid_t* out_uuid_v2
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (out_uuid_v2)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.master_uuid_offset),
				header->payload.master_uuid_size)
			== AUD_SUCCESS
			)
		{
			uint16_t valid_flags = ntohs(header->payload.valid_flags);
			// Cap the uuid size at 6 if validity flag is not set since older devices sent 6 valid bytes with size of 8
			uint8_t size = AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_BASIC2_VALID_FLAG__V2_UUID_FORMAT) ? header->payload.master_uuid_size : MIN(DDP_CLOCK_UUID_LENGTH, header->payload.master_uuid_size);
			return ddp_clock_basic2_response_read_uuid_eui64(heap_info._.array8, size, out_uuid_v2);
		}

		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}

aud_error_t
ddp_read_clock_basic2_response_grandmaster_uuid_v2
(
	const ddp_message_read_info_t * message_info,
	ddp_v2_clock_uuid_t* out_uuid_v2
)
{

	ddp_clock_basic2_response_t* header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (out_uuid_v2)
	{
		ddp_heap_read_info_t heap_info;
		if (
			ddp_read_heap_array8(
				message_info,
				&heap_info,
				ntohs(header->payload.grandmaster_uuid_offset),
				header->payload.grandmaster_uuid_size)
			== AUD_SUCCESS
			)
		{
			uint16_t valid_flags = ntohs(header->payload.valid_flags);

			// Cap the uuid size at 6 if validity flag is not set since older devices sent 6 valid bytes with size of 8
			uint8_t size = AUD_FLAG_ISSET(valid_flags, DDP_CLOCK_BASIC2_VALID_FLAG__V2_UUID_FORMAT) ? header->payload.grandmaster_uuid_size : MIN(DDP_CLOCK_UUID_LENGTH, header->payload.grandmaster_uuid_size);
			return ddp_clock_basic2_response_read_uuid_eui64(heap_info._.array8, size, out_uuid_v2);
		}

		return AUD_ERR_INVALIDDATA;
	}

	return AUD_ERR_INVALIDPARAMETER;

}
