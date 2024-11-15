/*
 * File     : switch_structures.h
 * Created  : December 2019
 * Updated  : Date: 2019/12/11
 * Author   : Savin Weeraratne <savin.weeraratne@audinate.com>
 * Synopsis : All switch register access DDP message structure definitions.
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_SWITCH_STRUCTURES_H
#define _DDP_SWITCH_STRUCTURES_H

#include "../ddp.h"

 /**
 * @file switch_structures.h
 * @brief All switch DDP messages structure definitions.
 */

 /** \addtogroup DDP
 *  @{
 */

 /** \addtogroup Switch
 *  @{
 */

 /**
 * \defgroup SwitchEnums DDP Switch Enums
 * @{
 */

/**
* @enum switch_access_operation
* @brief The required switch register access operation
*/
enum switch_access_operation
{
	DDP_SWITCH_REGISTER_READ = 1,	/*!< Perform a switch register read operation */
	DDP_SWITCH_REGISTER_WRITE,		/*!< Perform a switch register write operation */
};

/**@}*/

 /**
 * \defgroup SwitchStructs DDP Switch Structures
 * @{
 */

/**
* @struct ddp_switch_reg_access_request
* @brief Structure format for the "switch register access request" message
*/
struct ddp_switch_reg_access_request
{
	ddp_message_header_t header;			/*!< message header */
	ddp_request_subheader_t subheader;		/*!< request message subheader */

    struct
	{
        uint8_t phy_addr;				/*!< Phy_id to read/write */
        uint8_t phy_reg;            	/*!< Register number of phy to read/write */
		uint16_t phy_data;				/*!< Value to write to register */
		uint16_t rw_flag;				/*!< Flag to specify read or write operation */
		uint16_t padding;				/*!< Padding to 4-byte align */
    } payload; /*!< fixed payload */
};

/**
* @struct ddp_switch_reg_access_response
* @brief Structure format for the "switch register accesss response" message
*/
struct ddp_switch_reg_access_response
{
	ddp_message_header_t header;					/*!< message header */
	ddp_response_subheader_t subheader;				/*!< response message subheader */

	struct
	{
        uint16_t register_value;					/*!< Value of subject register after selected operation is completed */
		uint16_t padding;							/*!< Padding to 4-byte align */
    } payload; /*!< fixed payload */
};

//! @cond Doxygen_Suppress
typedef struct ddp_switch_reg_access_request ddp_switch_reg_access_request_t;
typedef struct ddp_switch_reg_access_response ddp_switch_reg_access_response_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif /* _DDP_SWITCH_STRUCTURES_H */
