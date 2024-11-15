/*
 * File     : ddp_shared.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types - inet address
 *
 * Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

//----------
// Include

#include "../include/ddp/ddp_shared.h"


//----------
// Transports


//----------
// Addresses


// Offset arrays
ddp_addr_inet_t *
ddp_addr_inet_allocate
(
	ddp_message_write_info_t * message_info,
	uint32_t in_addr,
		// network byte order
	uint16_t port
		// host byte order
);

ddp_addr_inet_t *
ddp_addr_inet_add_to_array
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint32_t in_addr,
		// network byte order
	uint16_t port
		// host byte order
);

//----------
