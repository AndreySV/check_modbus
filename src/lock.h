/*
  check_modbus: checker for Modbus TCP/RTU devices for Nagios control
  system based on libmodbus library

  Copyright (C) 2011 2012 2013 2014 Andrey Skvortsov <andrej.skvortzov@gmail.com>


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

*/

#ifndef _LOCK_H_
#define _LOCK_H_


#include "check_modbus.h"

enum {
	LOCK_INPUT,
	LOCK_OUTPUT
};


void  set_lock(struct modbus_params_t *params, int lock_type);
void  release_lock(struct modbus_params_t *params, int lock_type);

#endif
