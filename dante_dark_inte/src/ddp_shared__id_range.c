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
// ID range request

/*
	Allocate an id_ranges message.
	If max_ranges is non-zero, this sets an upper bound for the number of ranges.
	If it is zero, then the payload is treated like a heap and grows as ranges
	are added.
 */
aud_error_t
ddp_message_id_range_allocate_payload
(
	ddp_message_write_info_t * message_info,
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t max_ranges
)
{
	aud_error_t result;
	ddp_id_range_payload_t * p;
	ddp_id_ranges_t * ranges;
	ddp_size_t heap_bytes = sizeof(*ranges);

	assert(message_info && ranges_write);

	result = ddp_message_allocate_payload(message_info, sizeof(*p));
	if (result != AUD_SUCCESS)
		return result;
	p = (ddp_id_range_payload_t *) message_info->payload;
	p->pad0 = 0;

	if (max_ranges > 1)
	{
		heap_bytes += (ddp_size_t)sizeof(ddp_id_range_t) * (max_ranges - 1);
	}
	ranges = ddp_message_allocate_on_heap(message_info, &p->ranges_offset, heap_bytes);
	if (! ranges_write)
		return AUD_ERR_NOMEMORY;

	ranges_write->message_info = message_info;
	ranges_write->ranges = ranges;
	if (max_ranges)
	{
		ranges->max = htons(max_ranges);
	}

	return AUD_SUCCESS;
}


/*
	As add_range, but without compressing ranges or checking them for correctness
 */
aud_error_t
ddp_message_id_range_add_range
(
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t from, uint16_t to
)
{
	ddp_id_ranges_t * ranges;
	unsigned max, curr;

	assert(ranges_write && ranges_write->message_info && ranges_write->ranges);

	ranges = ranges_write->ranges;
	max = ddp_id_ranges_max(ranges);
	curr = ddp_id_ranges_curr(ranges);
	if (max)
	{
		if (curr >= max)
		{
			return AUD_ERR_NOMEMORY;
		}
	}
	else if (curr)
	{
		// if p->ranges.curr == 0, we have already allocated one range
		if (! ddp_message_write_grow_packet(ranges_write->message_info, sizeof(ddp_id_range_t)))
		{
			return AUD_ERR_NOMEMORY;
		}
	}

	ranges->range[curr].from = htons(from);
	ranges->range[curr].to = htons(to);
	ddp_id_ranges_set_curr(ranges, (uint16_t)curr+1);

	return AUD_SUCCESS;
}


const ddp_id_ranges_t *
ddp_message_id_range_validate
(
	const ddp_message_read_info_t * message_info
)
{
	if (message_info && message_info->payload.value32)
	{
		unsigned curr;

		const ddp_id_range_payload_t * r =
			(const ddp_id_range_payload_t *) message_info->payload.value16;
		ddp_size_t bytes = sizeof(*r);
		const ddp_id_ranges_t * ranges;

		if (! r->ranges_offset)
			return NULL;
		ranges = ddp_heap_resolve_sized(message_info, ntohs(r->ranges_offset), sizeof(*ranges));
		if (! ranges)
			return NULL;

		curr = ddp_id_ranges_curr(ranges);
		if (curr > 1)
		{
			bytes += (uint16_t)(curr - 1) * sizeof(ranges->range[0]);
		}
		if (ntohs(r->ranges_offset) + bytes <= message_info->length_bytes)
			return ranges;
		else
			return NULL;
	}
	else
		return NULL;
}


//----------
