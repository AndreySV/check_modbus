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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "../src/variant.h"

static int test_inverse(int format)
{
	struct data_t before, after;
	int size_words;
	int i;

	before.format = format;
	after.format  = format;

	/* sizeof_data_t returs size in words */
	size_words  = sizeof_data_t(&before);
	for (i = 0; i < size_words; i++) {
		before.val.words[i] = i;
		after.val.words[size_words-i-1] = i;
	}

	reorder_data_t(&before, 0, 1);
	return  equal_data_t(&before, &after);
}


int main(void)
{
	size_t i;
	int formats[] = {
		FORMAT_SIGNED_WORD,
		FORMAT_SIGNED_DWORD,
		FORMAT_SIGNED_QWORD
	};

	for (i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
		if (!test_inverse(formats[i]))
			return -1;
	}
	return 0;
}
