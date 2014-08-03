/*
  check_modbus: checker for Modbus TCP/RTU devices for Nagios control
  system based on libmodbus library

  Copyright (C) 2011 2012 2013 2014 Andrey Skvortsov


  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your  option)
  any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301, USA.


  Andrey Skvortsov
  Andrej . Skvortzov [at] gmail . com
*/

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "variant.h"

/* returns size in words */
int sizeof_data_t(struct data_t *data)
{
	int size = 0;

	switch (data->format) {
	case FORMAT_DUMP_BIN:
	case FORMAT_DUMP_HEX:
	case FORMAT_DUMP_DEC:
		size = data->arr_size;
		break;
	case FORMAT_SIGNED_WORD:
	case FORMAT_UNSIGNED_WORD:
		size = 1;
		break;
	case FORMAT_FLOAT:
	case FORMAT_SIGNED_DWORD:
	case FORMAT_UNSIGNED_DWORD:
		size = 2;
		break;
	case FORMAT_DOUBLE:
	case FORMAT_SIGNED_QWORD:
	case FORMAT_UNSIGNED_QWORD:
		size = 4;
		break;
	default:
		printf("Sizeof_data_t(): Unsupported format (%d)\n", data->format);
		break;
	}
	return size;
}



void clear_data_t(struct data_t *data)
{
	size_t i;

	for (i = 0; i < sizeof(data->val); i++)
		data->val.bytes[i] = 0;
}



void init_data_t(struct data_t *data, int8_t format, uint8_t size)
{
	clear_data_t(data);
	data->format = format;
	data->arr_size = size;
}



double value_data_t(struct data_t *data)
{
	double tmp;

	switch (data->format) {
	case FORMAT_SIGNED_WORD:
		tmp = data->val.sword;
		break;
	case FORMAT_UNSIGNED_WORD:
		tmp = data->val.word;
		break;
	case FORMAT_SIGNED_DWORD:
		tmp = data->val.sdword;
		break;
	case FORMAT_UNSIGNED_DWORD:
		tmp = data->val.dword;
		break;
	case FORMAT_SIGNED_QWORD:
		tmp = data->val.sqword;
		break;
	case FORMAT_UNSIGNED_QWORD:
		tmp = data->val.qword;
		break;
	case FORMAT_FLOAT:
		tmp = data->val.real;
		break;
	case FORMAT_DOUBLE:
		tmp = data->val.long_real;
		break;		
	default:
		tmp = 0;
	}
	return tmp;
}

void printf_data_t(FILE *fd, struct data_t *data)
{
	int i;

	switch (data->format) {
	case FORMAT_SIGNED_WORD:
		fprintf(fd, "%d", data->val.word);
		break;
	case FORMAT_UNSIGNED_WORD:
		fprintf(fd, "%u", data->val.word);
		break;
	case FORMAT_SIGNED_DWORD:
		fprintf(fd, "%"PRId32, data->val.sdword);
		break;
	case FORMAT_UNSIGNED_DWORD:
		fprintf(fd, "%"PRIu32, data->val.dword);
		break;
	case FORMAT_SIGNED_QWORD:
		fprintf(fd, "%"PRId64, data->val.sqword);
		break;
	case FORMAT_UNSIGNED_QWORD:
		fprintf(fd, "%"PRIu64, data->val.qword);
		break;
	case FORMAT_FLOAT:
		fprintf(fd, "%f", data->val.real);
		break;
	case FORMAT_DOUBLE:
		fprintf(fd, "%f", data->val.long_real);
		break;
	case FORMAT_DUMP_BIN:
		fwrite(data->val.words, sizeof(data->val.words[0]), data->arr_size, fd);
		break;
	case FORMAT_DUMP_HEX:
		for (i = 0; i < (2*data->arr_size);) {
			fprintf(fd, "%X ", data->val.bytes[i++]);
			if ((i%16) == 0)
				fprintf(fd, "\n");
		}
		fprintf(fd, "\n");
		break;
	case FORMAT_DUMP_DEC:
		for (i = 0; i < (2*data->arr_size);) {
			fprintf(fd, "%d ", data->val.bytes[i++]);
			if ((i%16) == 0)
				fprintf(fd, "\n");
		}
		fprintf(fd, "\n");
		break;

	default:
		fprintf(stderr, "Printf_struct data_t(): Unsupported format (%d)\n", data->format);
	}
}

static uint16_t swap_bytes(uint16_t word)
{
	return ((word & 0xFF00)>>8) | ((word & 0x00FF)<<8);
}


void reorder_data_t(struct data_t *data, int swap, int inverse_words)
{
	int	    size = sizeof_data_t(data);
	int	    i, j;
	uint16_t    word;
	struct data_t	    tmp;

	tmp  = *data;
	for (i = 0; i < size; i++) {
		j = inverse_words ? size-i-1 : i;
		word = tmp.val.words[j];
		data->val.words[i] = swap ? swap_bytes(word) : word;
	}

}

int equal_data_t(struct data_t *data1, struct data_t *data2)
{
	int	    size = sizeof_data_t(data1);
	int	    i;

	if (data1->format != data2->format)
		return 0;

	for (i = 0; i < size; i++)
		if (data1->val.bytes[i] != data2->val.bytes[i])
			return 0;

	return 1;
}
