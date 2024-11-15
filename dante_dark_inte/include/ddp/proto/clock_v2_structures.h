/*
* File     : clock_v2_structures.h
* Created  : July 2020
* Updated  : Date: 2020/07/09
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : All clock v2 DDP messages structure definitions.
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_CLOCK_V2_STRUCTURES_H
#define _DDP_CLOCK_V2_STRUCTURES_H

#include "../ddp.h"

/**
* @file v2_clocking_structures.h
* @brief All PTP v2 clocking ddp messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Clocking
*  @{
*/

/** \addtogroup ClockingEnums
*  @{
*/

/**
* @enum ddp_clock_v2_field_flags
* valid flags for the field_flags in the ddp_raw_clock_v2_payload
* These flags only exist if the payload is a request.
*/
enum
{
	DDP_CLOCK_V2_FIELD_FLAG__PRIORITY1_VALID = 1 << 0,		//!< priority1 field is valid in ddp_raw_clock_v2_payload
	DDP_CLOCK_V2_FIELD_FLAG__PRIORITY2_VALID = 1 << 1,		//!< priority2 field is valid in ddp_raw_clock_v2_payload
	DDP_CLOCK_V2_FIELD_FLAG__DOMAIN_NUMBER_VALID = 1 << 2,		//!< domain number field is valid in ddp_raw_clock_v2_payload
	DDP_CLOCK_V2_FIELD_FLAG__TTL_VALID = 1 << 3,			//!< TTL field is valid in ddp_raw_clock_v2_payload
	DDP_CLOCK_V2_FIELD_FLAG__SYNC_INTERVAL_VALID = 1 << 4,		//!< sync interval field is valid in ddp_raw_clock_v2_payload
	DDP_CLOCK_V2_FIELD_FLAG__ANNOUNCE_INTERVAL_VALID = 1 << 5,	//!< announce interval field is valid in ddp_raw_clock_v2_payload
};

/**
* @enum ddp_clock_v2_mode_flags
* mode flags in the ddp_raw_clock_v2_payload
*/
enum
{
	DDP_CLOCK_V2_MODE_FLAG__SMPTE = 1 << 0,	/*!< Enable v2 SMPTE mode clocking*/
};

/**
* @enum ddp_clock_v2_config_flags
* config flags in the ddp_raw_clock_v2_payload
*/
enum
{
	DDP_CLOCK_V2_CONFIG_FLAG__SLAVE_ONLY = 1 << 0,	/*!< Set clock as slave_only */
};

/**@}*/

/**
 * \addtogroup ClockingStructs
 * @{
 */

/**
* @struct ddp_raw_clock_v2_payload
* @brief Structure format for the "clock ptpv2" message
*/

struct ddp_raw_clock_v2_payload
{
	uint16_t mode_flags;			//!< Clock v2 mode flags @see ddp_clock_v2_mode_flags
	uint16_t mode_flags_mask;		//!< mask for mode flags
	uint32_t config_flags;			//!< Config flags @see ddp_clock_v2_config_flags.
	uint32_t config_flags_mask;		//!< mask for config flags
	uint32_t capability_flags;		//!< Capability flags. None defined yet. Ignored on requests
	uint32_t capability_flags_mask;		//!< mask for capability flags
	uint32_t status_flags;			//!< Status flags. None defined yet. Ignored on requests
	uint32_t status_flags_mask; 		//!< mask for status flags
	uint32_t field_flags;			//!< Field flags for this message @see ddp_clock_v2_field_flags.
	uint8_t priority1;			//!< Priority1 field of PTPv2
	uint8_t priority2;			//!< Priority2 field of PTPv2
	uint8_t domain_number;			//!< domain number field of PTPv2
	uint8_t v2_ttl;				//!< PTP v2 multicast port TTL value
	int8_t sync_interval;			//!< PTP v2 sync interval (value is log to the base 2)
	int8_t announce_interval;		//!< PTP v2 announce interval (value is log to the base 2)
	uint16_t padding;			//!< Set to zero.
};

//! @cond Doxygen_Suppress
typedef struct ddp_raw_clock_v2_payload ddp_raw_clock_v2_payload_t;
//! @endcond Doxygen_Suppress

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_CLOCK_V2_STRUCTURES_H
