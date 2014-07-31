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

#ifndef _COMPILE_DATE_TIME_
#define _COMPILE_DATE_TIME_

#define COMPILE_HOUR   (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0'))
#define COMPILE_MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define COMPILE_SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))


#define COMPILE_YEAR ((((__DATE__ [7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))

#define COMPILE_MONTH ((__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ?0 : 5) \
		      : __DATE__ [2] == 'b' ? 1 \
		      : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M'? 2 : 3) \
		      : __DATE__ [2] == 'y' ? 4 \
		      : __DATE__ [2] == 'l' ? 6 \
		      : __DATE__ [2] == 'g' ? 7 \
		      : __DATE__ [2] == 'p' ? 8 \
		      : __DATE__ [2] == 't' ? 9 \
		      : __DATE__ [2] == 'v' ? 10 : 11)+1)

#define COMPILE_DAY ((__DATE__ [4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))

#endif
