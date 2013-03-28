#ifndef _LOCK_H_
#define _LOCK_H_


#include "check_modbus.h"

enum
{
    LOCK_INPUT,
    LOCK_OUTPUT
};


void  set_lock(modbus_params_t* params, int lock_type);
void  release_lock(modbus_params_t* params, int lock_type);

#endif
