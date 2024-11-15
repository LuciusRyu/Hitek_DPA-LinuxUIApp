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

aud_error_t
ddp_array_init_read
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap,
	ddp_offset_t array_start_offset
)
{
	const ddp_array_t * array =
		ddp_heap_resolve_sized(message_info, array_start_offset, sizeof(ddp_array_t));

	heap->message_info = message_info;
	heap->offset_array = array;
	heap->_.array16 = (const uint16_t *) (const void *) (&array->header + 1);
	heap->num_elements = (uint16_t) ddp_array_curr(array);
	heap->element_size_bytes = sizeof(uint16_t);
	heap->length_bytes = (heap->num_elements - 1) * heap->element_size_bytes;

	if (array_start_offset + sizeof(array->header) > message_info->length_bytes)
		return AUD_ERR_RANGE;
	return AUD_SUCCESS;
}


/*
	Extract an offset from an array
 */
ddp_offset_t
ddp_array_get_offset
(
	ddp_heap_read_info_t * heap,
	unsigned index
)
{
	const ddp_array_t * array = (heap->offset_array);
	assert(array);
	if (index < ddp_array_curr(array))
	{
		return ntohs(array->offset[index]);
	}
	return 0;
}


const void *
ddp_offset_array_get_element
(
	ddp_heap_read_info_t * heap,
	unsigned index
)
{
	ddp_offset_t offset = ddp_array_get_offset(heap, index);
	if (offset)
	{
		return ddp_heap_resolve_sized(heap->message_info, offset, 0);
	}
	else
		return NULL;
}


/*
	Dereference an element of an offset array and check size

	If size > 0, then check that this size fits within message bounds.
 */
const void *
ddp_offset_array_read_element_fixed_size
(
	ddp_heap_read_info_t * heap,
	unsigned index,
	ddp_size_t size
)
{
	ddp_offset_t offset = ddp_array_get_offset(heap, index);
	if (! offset)
		return NULL;

	return ddp_heap_resolve_sized(heap->message_info, offset, size);
}


const void *
ddp_offset_array_read_element_self_sized
(
	ddp_heap_read_info_t * heap,
	unsigned index,
	ddp_size_t min_size
)
{
	ddp_offset_t offset = ddp_array_get_offset(heap, index);
	if (!offset)
		return NULL;

	const uint16_t * result =
		ddp_heap_resolve_sized(heap->message_info, offset, sizeof(*result));
	if (!result)
	{
		return NULL;
	}

	ddp_size_t size = ntohs(result[0]);
	if (offset + size > heap->message_info->length_bytes)
		return NULL;

	if (size < min_size)
		return NULL;

	return result;
}

// Offset array payload

aud_error_t
ddp_offset_array_payload_init_read
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap
)
{
	aud_error_t result;

	assert(message_info && heap);

	if (! message_info->payload.value16)
		return AUD_ERR_INVALIDPARAMETER;

	result = ddp_array_init_read(message_info, heap, ntohs(message_info->payload.value16[0]));
	return result;
}


//----------
