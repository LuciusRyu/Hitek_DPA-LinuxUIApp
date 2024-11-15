/*
 * File     : ddp_shared.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types
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
// Offset array allocate string

/*
	Allocate a string on the heap and assign the pointer to the next
	empty spot in the array.

	Returns NULL if allocation failed or there were no elements free in the array.
	NULL string is consdered a failure.
 */
char *
ddp_array_allocate_string
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	const char * string
)
{
	uint16_t n;
	char * result;
	assert (message_info && array);

	if (! string)
		return NULL;

	n = ddp_array_curr(array);
	if (n >= ddp_array_max(array))
		return NULL;

	result = ddp_message_allocate_string(message_info, &array->offset[n], string);
	if (result)
		ddp_array_set_curr(array, n+1);
	return result;
}


//----------
