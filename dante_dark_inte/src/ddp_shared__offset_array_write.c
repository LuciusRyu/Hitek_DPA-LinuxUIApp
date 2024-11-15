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
// Offset array

ddp_array_t *
ddp_array_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	unsigned n
)
{
	ddp_array_t * array;
	ddp_size_t array_bytes;

	if (! n)
		return NULL;

	array_bytes = (ddp_size_t) (sizeof(ddp_array_t) + (n - 1) * sizeof(array->offset[0]));
	array = ddp_message_allocate_on_heap(message_info, offset, array_bytes);
	if (array)
	{
		array->header.max = htons((uint16_t) n);
	}
	return array;
}


/*
	Allocate heap_bytes of data on the heap and assign the pointer to the next
	empty spot in the array.

	Returns NULL if allocation failed or there were no elements free in the array.
 */
void *
ddp_array_allocate_element
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	ddp_size_t heap_bytes
)
{
	uint16_t n;
	void * result;
	assert (message_info && array);

	if (! heap_bytes)
		return NULL;

	n = ddp_array_curr(array);
	if (n >= ddp_array_max(array))
		return NULL;

	result = ddp_message_allocate_on_heap(message_info, &array->offset[n], heap_bytes);
	if (result)
		ddp_array_set_curr(array, n+1);
	return result;
}


// Offset array payload

ddp_array_t *
ddp_offset_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	unsigned n
)
{
	ddp_offset_array_payload_t * p;
	if (!(message_info && n))
		return NULL;

	if (ddp_message_allocate_payload(message_info, sizeof(*p)) != AUD_SUCCESS)
	{
		return NULL;
	}

	p = (ddp_offset_array_payload_t *) message_info->payload;
	return ddp_array_allocate(message_info, &p->array_offset, n);
}

aud_error_t
ddp_array_remove_last_element
(
	ddp_array_t * array
)
{
	uint16_t n;
	assert(array);
	n = ddp_array_curr(array);
	if (!n)
		return AUD_ERR_NOTFOUND;

	array->offset[n - 1] = 0;
	ddp_array_set_curr(array, n - 1);
	return AUD_SUCCESS;
}

//----------
