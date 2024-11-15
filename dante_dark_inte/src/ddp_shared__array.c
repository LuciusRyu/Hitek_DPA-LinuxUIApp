/*
 * File     : ddp_shared.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types - array
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
// Array payload

aud_error_t
ddp_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_size_t element_size
)
{
	aud_error_t result;
	ddp_array_payload_t * r;

	assert(message_info);
	if (! element_size)
		return AUD_ERR_INVALIDPARAMETER;

	result = ddp_message_allocate_payload(message_info, sizeof(*r));
	if (result != AUD_SUCCESS)
		return result;

	r = (ddp_array_payload_t *) message_info->payload;
	r->element_size = htons(element_size);
	r->num_elements = 0;
	r->array_offset = 0;
	r->pad0 = 0;
	return AUD_SUCCESS;
}


void *
ddp_array_payload_allocate_elements
(
	ddp_message_write_info_t * message_info,
	uint16_t num_elements
)
{
	ddp_array_payload_t * r;
	uint8_t * array;
	ddp_size_t heap_bytes, elem_size;

	assert(message_info && message_info->payload);
	if (! num_elements)
		return NULL;

	r = (ddp_array_payload_t *) message_info->payload;
	elem_size = ntohs(r->element_size);
	heap_bytes = num_elements * elem_size;

	if (!r->array_offset)
	{
		array = ddp_message_allocate_on_heap(message_info, &r->array_offset, heap_bytes);
		if (array)
		{
			r->num_elements = htons(num_elements);
		}
	}
	else
	{
		uint16_t curr_elements = ntohs(r->num_elements);
		array = ((uint8_t *)message_info->_.buf32) + ntohs(r->array_offset);
		array += (curr_elements * elem_size);
		if (! ddp_message_write_grow_packet(message_info, heap_bytes))
			return NULL;

		bzero(array, heap_bytes);
		curr_elements += num_elements;
		r->num_elements = htons(curr_elements);
	}
	return (void *) array;
}


//----------
