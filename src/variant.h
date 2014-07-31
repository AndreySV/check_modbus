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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301, USA.


  Andrey Skvortsov
  Andrej . Skvortzov [at] gmail . com
*/

#ifndef _VARIANT_H_
#define _VARIANT_H_

#include <stdint.h>

typedef struct {
	union {
		uint8_t     bytes[256];
		uint16_t    words[128];

		uint8_t     byte;
		int8_t      sbyte;

		uint16_t    word;
		int16_t     sword;

		uint32_t    dword;
		int32_t     sdword;

		uint64_t    qword;
		int64_t     sqword;

		float       real;
		double      long_real;
	} val;
	int8_t      format;
	uint8_t     arr_size;
} data_t;


enum {
	FORMAT_MIN_SUPPORTED = 0,
	FORMAT_SIGNED_WORD,
	FORMAT_UNSIGNED_WORD,
	FORMAT_SIGNED_DWORD,
	FORMAT_UNSIGNED_DWORD,
	FORMAT_SIGNED_QWORD,
	FORMAT_UNSIGNED_QWORD,
	FORMAT_FLOAT,
	FORMAT_DOUBLE,
	FORMAT_DUMP_BIN,
	FORMAT_DUMP_HEX,
	FORMAT_DUMP_DEC,
	FORMAT_MAX,
	FORMAT_MAX_SUPPORTED = FORMAT_DOUBLE+1,
	FORMAT_DUMP_MIN = FORMAT_DUMP_BIN-1,
	FORMAT_DUMP_MAX = FORMAT_DUMP_DEC+1
};



int     sizeof_data_t(data_t *data);    /* returns size in words */
void    printf_data_t(FILE *fd, data_t *data);
double  value_data_t(data_t *data);
void    init_data_t(data_t *data, int8_t format, uint8_t size);
void    clear_data_t(data_t *data);
void    reorder_data_t(data_t *data, int swap_bytes, int inverse_words);
int     equal_data_t(data_t *data1, data_t *data2);

#endif
