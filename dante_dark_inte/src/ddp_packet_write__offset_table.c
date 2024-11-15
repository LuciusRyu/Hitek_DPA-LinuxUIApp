/*
* File     : ddp_packet_write.c
* Created  : March 2019
* Author   : Andrew White <andrew.white@audinate.com>
* Synopsis : DDP packet write functions for offset tables
*
* Copyright 2005-2021, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "../include/ddp/packet.h"


//----------------------------------------------------------
// Data aggregation
//----------------------------------------------------------

/*
	Data aggregation allows a single item of data to represent multiple
	strings, with all offsets pointing to the same value.

	To solve in the general case, we keep a table of all strings (or byte-bufs)
	and their matching offsets and compare each allocation to the existing data.
 */

const void *
ddp_message_write_offset_table_get_entry
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const void * data,
	ddp_size_t data_len
)
{
	unsigned i;
	if (data_len)
	{
		for (i = 0; i < table->head.curr; i++)
		{
			if (table->entry[i].len != data_len)
				continue;

			if (memcmp(table->entry[i].data.data, data, data_len) == 0)
			{
				* offset_out = table->entry[i].offset;
				return table->entry[i].data.data;
			}
		}
	}
	else
	{
		for (i = 0; i < table->head.curr; i++)
		{
			if (strcmp(table->entry[i].data.string, data) == 0)
			{
				* offset_out = table->entry[i].offset;
				return table->entry[i].data.data;
			}
		}
	}

	return 0;
}

const char *
ddp_message_write_offset_table_get_strcase
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	unsigned i;
	for (i = 0; i < table->head.curr; i++)
	{
		if (strcasecmp(table->entry[i].data.string, string) == 0)
		{
			* offset_out = table->entry[i].offset;
			return table->entry[i].data.string;
		}
	}

	return 0;
}


const void *
ddp_message_write_offset_table_allocate_data
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const void * data,
	ddp_size_t size
)
{
	if (table)
	{
		const void * data_out = ddp_message_write_offset_table_get_entry(table, offset_out, data, size);
		if (data_out)
		{
			return data_out;
		}
		else if (table->head.curr < table->head.max)
		{
			ddp_message_write_offset_table_entry_t * entry = table->entry + table->head.curr;
			entry->data.data = ddp_message_allocate_data(msg_info, &entry->offset, size, data);
			if (entry->data.data)
			{
				table->head.curr++;
				*offset_out = entry->offset;
				return entry->data.data;
			}
		}
		return NULL;
	}
	else
	{
		return ddp_message_allocate_data(msg_info, offset_out, size, data);
	}
}


static const char *
ddp_message_write_offset_table_allocate_string_common
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string,
	aud_bool_t check_case
)
{
	if (table)
	{
		const char * string_out;
		if (check_case)
			string_out = ddp_message_write_offset_table_get_string(table, offset_out, string);
		else
			string_out = ddp_message_write_offset_table_get_string(table, offset_out, string);
		if (string_out)
		{
			return string_out;
		}
		else if (table->head.curr < table->head.max)
		{
			ddp_message_write_offset_table_entry_t * entry = table->entry + table->head.curr;
			entry->data.string = ddp_message_allocate_string(msg_info, &entry->offset, string);
			if (entry->data.string)
			{
				table->head.curr++;
				*offset_out = entry->offset;
				return entry->data.string;
			}
		}
		return NULL;
	}
	else
	{
		return ddp_message_allocate_string(msg_info, offset_out, string);
	}
}

const char *
ddp_message_write_offset_table_allocate_string
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	return ddp_message_write_offset_table_allocate_string_common(
		msg_info, table, offset_out, string, AUD_TRUE
	);
}


const char *
ddp_message_write_offset_table_allocate_string_case
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	return ddp_message_write_offset_table_allocate_string_common(
		msg_info, table, offset_out, string, AUD_FALSE
	);
}


//----------------------------------------------------------
