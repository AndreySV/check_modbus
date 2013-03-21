#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_

#include "check_modbus.h"

int     parse_command_line(modbus_params_t* params, int argc, char **argv);
void    print_settings(FILE* fd, modbus_params_t* params);

#endif
