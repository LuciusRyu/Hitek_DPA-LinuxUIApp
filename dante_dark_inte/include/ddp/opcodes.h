/*
* File     : opcodes.h
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : DDP message opcodes
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_OPCODES_H
#define _DDP_OPCODES_H

//----------
// OPCODES

#define DDP_OP_DEVICE_BASE			0x1000
#define DDP_OP_DEVICE_END			0x10FF

#define DDP_OP_NETWORK_BASE			0x1100
#define DDP_OP_NETWORK_END			0x11FF

#define DDP_OP_CLOCK_BASE			0x1200
#define DDP_OP_CLOCK_END			0x12FF
#define DDP_OP_CLOCK_RESERVE		0x12A0	//opcodes 0x12A0-0x12FF are reserved*/

#define DDP_OP_ROUTING_BASE			0x1300
#define DDP_OP_ROUTING_END			0x13FF

#define DDP_OP_AUDIO_BASE			0x1400
#define DDP_OP_AUDIO_END			0x14FF
#define DDP_OP_AUDIO_RESERVE		0x14A0	// opcodes 0x14A0-0x14FF are reserved.

#define DDP_OP_MDNS_BASE			0x1500
#define DDP_OP_MDNS_END				0x15FF

#define DDP_OP_SWITCH_BASE                      0x1600
#define DDP_OP_SWITCH_END                       0x16FF

// Range 0x1800 - 0x1FFF is reserved
#define DDP_OP_RESERVED_BIT_01 0x800


#define DDP_OP_RESERVED_BASE		0x2000
#define DDP_OP_RESERVED_END			0x2FFF

#define DDP_OP_LOCAL_BASE			0x4000
#define DDP_OP_LOCAL_END			0x40FF

#define DDP_OP_MONITOR_BASE			0x8000
#define DDP_OP_MONITOR_END			0x80FF

// Device message opcodes
typedef enum
{
	DDP_DEVICE_TYPE_GENERAL,
	DDP_DEVICE_TYPE_MANF,
	DDP_DEVICE_TYPE_SECURITY,
	DDP_DEVICE_TYPE_UPGRADE,
	DDP_DEVICE_TYPE_ERASE_CONFIG,
	DDP_DEVICE_TYPE_REBOOT,
	DDP_DEVICE_TYPE_IDENTITY,
	DDP_DEVICE_TYPE_IDENTIFY,
	DDP_DEVICE_TYPE_GPIO,
	DDP_DEVICE_TYPE_SWITCH_LED,
	DDP_DEVICE_TYPE_AES67,
	DDP_DEVICE_TYPE_LOCK_UNLOCK,
	DDP_DEVICE_TYPE_SWITCH_REDUNDANCY,
	DDP_DEVICE_TYPE_UART_CONFIG,
	DDP_DEVICE_TYPE_VLAN_CONFIG,
	DDP_DEVICE_TYPE_METER_CONFIG,
	DDP_DEVICE_TYPE_DANTE_DOMAIN,
	DDP_DEVICE_TYPE_SWITCH_STATUS,
	DDP_DEVICE_TYPE_CONFIG_REVISION,
	DDP_DEVICE_TYPE_READ_CONFIG,
	DDP_DEVICE_TYPE_END_BACKUP,
	DDP_DEVICE_TYPE_WRITE_CONFIG,
	DDP_DEVICE_TYPE_CONFIG_PKT_ACK,
	DDP_DEVICE_TYPE_CONFIG_PKT_NACK,
	DDP_DEVICE_TYPE_END_RESTORE,
	DDP_DEVICE_TYPE_PEER_GROUP_ID,
	DDP_NUM_DEVICE_TYPES
} ddp_device_type_t;

// network message opcodes
typedef enum
{
	DDP_NETWORK_TYPE_BASIC,
	DDP_NETWORK_TYPE_CONFIG,
	DDP_NUM_NETWORK_TYPES
} ddp_network_type_t;

// clocking message opcodes
typedef enum
{
	DDP_CLOCK_TYPE_BASIC_LEGACY,
	DDP_CLOCK_TYPE_CONFIG,
	DDP_CLOCK_TYPE_PULLUP,
	DDP_CLOCK_TYPE_BASIC2,
	DDP_CLOCK_TYPE_V2_QUERY,
	DDP_NUM_CLOCK_TYPES,
} ddp_clock_type_t;

// Routing message opcodes
typedef enum
{
	DDP_ROUTING_TYPE_BASIC,
	DDP_ROUTING_TYPE_READY_STATE,
	DDP_ROUTING_TYPE_PERFORMANCE_CONFIG,

	DDP_ROUTING_TYPE_RX_CHAN_CONFIG_STATE,
	DDP_ROUTING_TYPE_TX_CHAN_CONFIG_STATE,
	DDP_ROUTING_TYPE_RX_FLOW_CONFIG_STATE,
	DDP_ROUTING_TYPE_TX_FLOW_CONFIG_STATE,

	DDP_ROUTING_TYPE_RX_CHAN_STATUS,
	DDP_ROUTING_TYPE_RX_FLOW_STATUS,

	DDP_ROUTING_TYPE_RX_SUBSCRIBE_SET,
	DDP_ROUTING_TYPE_RX_UNSUB_CHAN,
	DDP_ROUTING_TYPE_RX_CHAN_LABEL_SET,
	DDP_ROUTING_TYPE_TX_CHAN_LABEL_SET,

	DDP_ROUTING_TYPE_MCAST_TX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_MANUAL_RX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_MANUAL_TX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_FLOW_DELETE,

	DDP_ROUTING_TYPE_RTP_RX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_RTP_TX_FLOW_CONFIG_SET,

	DDP_NUM_ROUTING_TYPES
} ddp_routing_type_t;

// audio message opcodes
typedef enum
{
	DDP_AUDIO_TYPE_BASIC,
	DDP_AUDIO_TYPE_SRATE_CONFIG,
	DDP_AUDIO_TYPE_ENC_CONFIG,
	DDP_AUDIO_TYPE_ERROR,
	DDP_AUDIO_TYPE_SIGNAL_PRESENCE_CONFIG,
	DDP_AUDIO_TYPE_SIGNAL_PRESENCE_DATA,
	DDP_AUDIO_TYPE_INTERFACE,
	DDP_AUDIO_TYPE_RTP_QUERY,
	DDP_NUM_AUDIO_TYPES
} ddp_audio_type_t;

// mdns message opcodes
typedef enum
{
	DDP_MDNS_TYPE_CONFIG,
	DDP_MDNS_TYPE_REGISTER_SERVICE,
	DDP_MDNS_TYPE_DEREGISTER_SERVICE,
	DDP_NUM_MDNS_TYPES
} ddp_mdns_type_t;

// switch message opcodes
typedef enum
{
	DDP_SWITCH_TYPE_REG_ACCESS
} ddp_switch_type_t;

//local message opcodes
typedef enum
{
	DDP_LOCAL_TYPE_AUDIO_FORMAT,
	DDP_LOCAL_TYPE_CLOCK_PULLUP,
	DDP_NUM_LOCAL_TYPES
} ddp_local_type_t;

// Monitoring message opcodes are packed and incremental
// to make it easy to describe expected messages via a bitmask
typedef enum
{
	DDP_MONITORING_TYPE_INTERFACE_STATISTICS,
	DDP_MONITORING_TYPE_CLOCK,
	DDP_MONITORING_TYPE_SIGNAL_PRESENCE,
	DDP_MONITORING_TYPE_RXFLOW_MAX_LATENCY,
	DDP_MONITORING_TYPE_RXFLOW_LATE_PACKETS,
	DDP_MONITORING_TYPE_TIMER_ACCURACY, // Deprecated, do not use. See PCS-2293 for details
	DDP_MONITORING_TYPE_TIMER_STATS,
	DDP_MONITORING_TYPE_RXFLOW_MAX_INTERVAL,
	DDP_MONITORING_TYPE_TRACING,
	DDP_NUM_MONITORING_TYPES

	//DDP_MONITORING_TYPE_RXFLOW_EARLY_PACKETS,
	//DDP_MONITORING_TYPE_RXFLOW_OUT_OF_ORDER_PACKETS,
	//DDP_MONITORING_TYPE_RXFLOW_DROPPED_PACKETS,

} ddp_monitoring_type_t;

// DDP opcodes
enum
{
	DDP_OP_NONE = 0x0000,

	DDP_OP_PACKET_HEADER = 0x0001,

	DDP_OP_EMPTY = 0x0002,
		// Indicates a block with no content - only the block header is meaningful
		// Should be skipped during packet parsing
		// Useful to blank out a message in an existing packet

	//device opcodes = base 0x1000
	DDP_OP_DEVICE_GENERAL						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_GENERAL,
	DDP_OP_DEVICE_MANF							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_MANF,
	DDP_OP_DEVICE_SECURITY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SECURITY,
	DDP_OP_DEVICE_UPGRADE						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_UPGRADE,
	DDP_OP_DEVICE_ERASE_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_ERASE_CONFIG,
	DDP_OP_DEVICE_REBOOT						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_REBOOT,
	DDP_OP_DEVICE_IDENTITY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_IDENTITY,
	DDP_OP_DEVICE_IDENTIFY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_IDENTIFY,
	DDP_OP_DEVICE_GPIO							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_GPIO,
	DDP_OP_DEVICE_SWITCH_LED					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_LED,
	DDP_OP_DEVICE_AES67							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_AES67,
	DDP_OP_DEVICE_LOCK_UNLOCK					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_LOCK_UNLOCK,
	DDP_OP_DEVICE_SWITCH_REDUNDANCY				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_REDUNDANCY,
	DDP_OP_DEVICE_UART_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_UART_CONFIG,
	DDP_OP_DEVICE_VLAN_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_VLAN_CONFIG,
	DDP_OP_DEVICE_METER_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_METER_CONFIG,
	DDP_OP_DEVICE_DANTE_DOMAIN					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_DANTE_DOMAIN,
	DDP_OP_DEVICE_SWITCH_STATUS					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_STATUS,
	DDP_OP_DEVICE_CONFIG_REVISION				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_CONFIG_REVISION,
	DDP_OP_DEVICE_READ_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_READ_CONFIG,
	DDP_OP_DEVICE_CONFIG_END_BACKUP				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_END_BACKUP,
	DDP_OP_DEVICE_WRITE_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_WRITE_CONFIG,
	DDP_OP_DEVICE_CONFIG_PKT_ACK				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_CONFIG_PKT_ACK,
	DDP_OP_DEVICE_CONFIG_PKT_NACK				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_CONFIG_PKT_NACK,
	DDP_OP_DEVICE_CONFIG_END_RESTORE			= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_END_RESTORE,
	DDP_OP_DEVICE_PEER_GROUP_ID					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_PEER_GROUP_ID,

	//network opcodes = base 0x1100
	DDP_OP_NETWORK_BASIC						= DDP_OP_NETWORK_BASE + DDP_NETWORK_TYPE_BASIC,
	DDP_OP_NETWORK_CONFIG						= DDP_OP_NETWORK_BASE + DDP_NETWORK_TYPE_CONFIG,

	//clocking opcodes = base 0x1200
	DDP_OP_CLOCK_BASIC_LEGACY					= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_BASIC_LEGACY,
	DDP_OP_CLOCK_CONFIG							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_CONFIG,
	DDP_OP_CLOCK_PULLUP							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_PULLUP,
	DDP_OP_CLOCK_BASIC2							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_BASIC2,
	DDP_OP_CLOCK_V2_QUERY						= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_V2_QUERY,

	//routing opcodes = base 0x1300
	DDP_OP_ROUTING_BASIC						= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_BASIC,
	DDP_OP_ROUTING_READY_STATE					= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_READY_STATE,
	DDP_OP_ROUTING_PERFORMANCE_CONFIG			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_PERFORMANCE_CONFIG,
	DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_CONFIG_STATE,
	DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_CHAN_CONFIG_STATE,
	DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_FLOW_CONFIG_STATE,
	DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_FLOW_CONFIG_STATE,
	DDP_OP_ROUTING_RX_CHAN_STATUS				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_STATUS,
	DDP_OP_ROUTING_RX_FLOW_STATUS				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_FLOW_STATUS,
	DDP_OP_ROUTING_RX_SUBSCRIBE_SET				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_SUBSCRIBE_SET,
	DDP_OP_ROUTING_RX_UNSUB_CHAN				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_UNSUB_CHAN,
	DDP_OP_ROUTING_RX_CHAN_LABEL_SET			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_LABEL_SET,
	DDP_OP_ROUTING_TX_CHAN_LABEL_SET			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_CHAN_LABEL_SET,
	DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET		= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MCAST_TX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_MANUAL_RX_FLOW_CONFIG_SET	= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MANUAL_RX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_MANUAL_TX_FLOW_CONFIG_SET	= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MANUAL_TX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_RTP_RX_FLOW_CONFIG_SET		= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RTP_RX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_RTP_TX_FLOW_CONFIG_SET		= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RTP_TX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_FLOW_DELETE					= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_FLOW_DELETE,

	//audio opcodes = base 0x1400
	DDP_OP_AUDIO_BASIC							= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_BASIC,
	DDP_OP_AUDIO_SRATE_CONFIG					= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SRATE_CONFIG,
	DDP_OP_AUDIO_ENC_CONFIG						= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_ENC_CONFIG,
	DDP_OP_AUDIO_ERROR							= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_ERROR,
	DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG			= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SIGNAL_PRESENCE_CONFIG,
	DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA			= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SIGNAL_PRESENCE_DATA,
	DDP_OP_AUDIO_INTERFACE						= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_INTERFACE,
	DDP_OP_AUDIO_RTP_QUERY						= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_RTP_QUERY,

	//mdns opcodes = base 0x1500
	DDP_OP_MDNS_CONFIG							= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_CONFIG,
	DDP_OP_MDNS_REGISTER_SERVICE				= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_REGISTER_SERVICE,
	DDP_OP_MDNS_DEREGISTER_SERVICE				= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_DEREGISTER_SERVICE,

	//switch opcodes = base 0x1600
	DDP_OP_SWITCH_REG_ACCESS				= DDP_OP_SWITCH_BASE + DDP_SWITCH_TYPE_REG_ACCESS,

	//local opcodes = base 0x4000
	DDP_OP_LOCAL_AUDIO_FORMAT					= DDP_OP_LOCAL_BASE + DDP_LOCAL_TYPE_AUDIO_FORMAT,
	DDP_OP_LOCAL_CLOCK_PULLUP					= DDP_OP_LOCAL_BASE + DDP_LOCAL_TYPE_CLOCK_PULLUP,

	//monitoring opcodes = base 0x8000
	DDP_OP_MONITOR_INTERFACE_STATISTICS			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_INTERFACE_STATISTICS,
	DDP_OP_MONITOR_CLOCK						= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_CLOCK,
	DDP_OP_MONITOR_SIGNAL_PRESENCE				= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_SIGNAL_PRESENCE,
	DDP_OP_MONITOR_RXFLOW_MAX_LATENCY			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_RXFLOW_MAX_LATENCY,
	DDP_OP_MONITOR_RXFLOW_LATE_PACKETS			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_RXFLOW_LATE_PACKETS,
	DDP_OP_MONITOR_TIMER_ACCURACY				= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_TIMER_ACCURACY,
	DDP_OP_MONITOR_TIMER_STATS         			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_TIMER_STATS,
	DDP_OP_MONITOR_RXFLOW_MAX_INTERVAL 			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_RXFLOW_MAX_INTERVAL,
	DDP_OP_MONITOR_TRACING 						= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_TRACING
};



#endif // _DDP_OPCODES_H
