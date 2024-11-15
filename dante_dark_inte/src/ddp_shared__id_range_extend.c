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

aud_bool_t
ddp_range_extend_range
(
	ddp_id_range_t * range,
	uint16_t from, uint16_t to
)
{
	uint16_t range_from = ntohs(range->from);
	uint16_t range_to = ntohs(range->to);

	if (! (ddp_is_valid_range(from, to) && ddp_is_valid_range(range_from, range_to)))
		return AUD_FALSE;

	if (from == 0)
		return AUD_TRUE;
			// don't need to test to == 0 since that's implied by previous check
	if (range_from == 0)
	{
		range->from = htons(from);
		range->to = htons(to);
		return AUD_TRUE;
	}

	// At this point, we have two valid non-empty ranges
	// Do they overlap?
	if (to && range_from > to + 1)
		return AUD_FALSE;
	if (range_to && from > range_to + 1)
		return AUD_FALSE;

	// overlap - just figure bounds
	if (range_to)
	{
		if (! to)
		{
			range_to = 0;
		}
		else if (to > range_to)
		{
			range_to = htons(to);
		}
	}
	if (from < range_from)
	{
		range->from = htons(from);
	}
	return AUD_TRUE;
}


/*
	Add another range to an existing message, initialising the payload if
	necessary.
	Where possible, ranges are automatically compressed: adding range 2-5 to
	range 1-3 will result in a single range 1-5.  This is only applied to the
	last range in the list.
 */
aud_error_t
ddp_message_id_range_extend_range
(
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t from, uint16_t to
)
{
	ddp_id_ranges_t * ranges;
	unsigned curr;

	assert(ranges_write && ranges_write->message_info && ranges_write->ranges);

	if (from == 0 && to == 0)
	{
		return AUD_SUCCESS;
	}

	if (! ddp_is_valid_range(from, to))
	{
		return AUD_ERR_RANGE;
	}

	ranges = ranges_write->ranges;
	curr = ddp_id_ranges_curr(ranges);
	if (curr)
	{
		ddp_id_range_t * last_range = ranges->range + (curr - 1);
		if (ddp_range_extend_range(last_range, from, to))
			return AUD_SUCCESS;
	}

	return ddp_message_id_range_add_range(ranges_write, from, to);
}


//----------
