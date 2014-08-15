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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dbg_printf.h"


static int dbg_level;

void dbg_set_level(int level)
{
	dbg_level = level;
}

int dbg_get_level(void)
{
	return dbg_level;
}

bool dbg_chk_level(int level)
{
	return (dbg_level >= level);
}


static char *alloc_vprintf(const char *fmt, va_list arg)
{
	va_list arg_copy;
	int len;
	char *str;

	va_copy(arg_copy, arg);
	len = vsnprintf(NULL, 0, fmt, arg_copy);
	va_end(arg_copy);	
	if (len < 0)
		return NULL;
	
	/* additional byte for the NULL terminator */
	len++;
	
	str = (char*)malloc(len);
	if (str != NULL)
		vsnprintf(str, len, fmt, arg);

	return str;
}


void dbg_printf(int level,
		const char *file,
		unsigned int line,
		const char *function,
		const char *format,
		...)
{
	char *str;
	va_list arg;
	FILE* fd;

	if (level > dbg_level)
		return;

	fd = (level == DBG_ERROR) ? stderr : stdout;
		
	va_start(arg, format);

	str = alloc_vprintf(format, arg);
	if (str != NULL) {
		fprintf(fd, "%s:%d:%s(): %s\n", file, line, function, str);
		free(str);
	}

	va_end(arg);
}
