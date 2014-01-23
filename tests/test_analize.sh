#!/bin/bash

#******************************************************************************
# check_modbus: checker for Modbus TCP/RTU devices for Nagios control
# system based on libmodbus library

# Copyright (C) 2011 2012 2013 2014 Andrey Skvortsov

  
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 3, or (at your  option)  
# any later version.
  
# This program is distributed in the hope that it will be useful, but  
# WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU    
# General Public License for more details.
  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software  
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301, USA. 


# Andrey Skvortsov
# Andrej . Skvortzov [at] gmail . com
#******************************************************************************



#*********************************************************
# name of function for with testcase should have name 
#
#        test_[any string][_disable]
#
# tests with "_disable" string in the name
# will not be executed
#*********************************************************

function test_check_modbus
{
    local rc_need=$1
    local params="$2"

    check_modbus -v --file $DUMP_FILE_SRC --lock_file_out $LOCK_FILE  -f 3 $params  >> $TMP_LOG_FILE 2>&1
    local rc=$?

    if [ $rc -ne $rc_need ]; then
        echo "failed $FUNCNAME(): rc = $rc, but expected $rc_need" >> $TMP_LOG_FILE
        return 1
    fi
    return 0
}




#*********************************************************


function run_all_test
{
    ret=0    
    echo > $LOG_FILE # clear log file
    

    for i in $( seq ${num:=0} ); do 
        
        echo > $TMP_LOG_FILE # clear log file
        echo '------------------------------------' >> $TMP_LOG_FILE
        echo "Test number:  $i  " >> $TMP_LOG_FILE
        echo $0 >> $TMP_LOG_FILE    
        test_check_modbus "${arr_rc[$i]}"  "${arr_params[$i]}"
        if [ $? -ne 0 ]; then
            ret=1
            cat $TMP_LOG_FILE >> $LOG_FILE
        fi

    done;
    
    # finish testing

    rm $TMP_LOG_FILE >> /dev/null
    if [ $ret -ne 0 ]; then
        echo "Some tests failed. For additional information see $LOG_FILE"
    else
        rm  $LOG_FILE >> /dev/null
    fi
    return $ret
}



function add_test()
{
    local rc=$1
    local params="$2"

    if [ "$rc"     == "" ]; then return; fi
    if [ "$params" == "" ]; then return; fi;

    num=$(( ${num:=0}  + 1 ))
    arr_params[$num]="$params"
    arr_rc[$num]=$rc
}

#******************************************************************************
# settings
#******************************************************************************

TEST_NAME=$( basename $0 .sh)

# path to compiled binary
PATH=../src/:$PATH  


# VERBOSE=1
DUMP_FILE_SRC=${TEST_NAME}_dump.bin
LOG_FILE=${TEST_NAME}.log
LOCK_FILE=${TEST_NAME}.lock
TMP_LOG_FILE=${TEST_NAME}_tmp.log


# fill tests
add_test 0 '-a 1 -N'
add_test 2 '-a 1 -n'
add_test 1 '-a 3 -w 3 -c 7'
add_test 1 '-a 3 -w 7 -c 3'
add_test 2 '-a 3 -w 1 -c 3'
add_test 0 '-a 3 -w 3 -c 1'
add_test 1 '-a 6 -w 100 -c 102 -F 7 -i -s'
add_test 1 '-a 8 -w 100 -c 102 -F 7'
add_test 1 '-a 10 -w 100 -c 102 -F 7 -s'
# add_test 1 '-a 12 -w 100 -c 102 -F 8 -i -s'
# add_test 1 '-a 16 -w 100 -c 102 -F 8'

# check software
run_all_test
exit $?
