#!/bin/bash

check_software()
{
    local missing=0
    
    if ! which bc>/dev/null; then
        echo "bc command is missing"
        missing=1
    fi

    if ! which awk>/dev/null; then
        echo "awk command is missing"
        missing=1
    fi
    
    if ! which awk>/dev/null; then
        echo "check_modbus command is missing"
        missing=1
    fi

    
    if [ $missing -ne 0 ]; then
        exit -10;
    fi
    
}

print_settings()
{
    echo "IP:       <$IP>"
    echo "REG_ADDR: <$REG_ADDR>"
    echo "WARNING:  <$WARNING>"
    echo "CRITICAL: <$CRITICAL>"
}


read_modbus()
{
    # read modbus value
    result=$(check_modbus --ip=$IP -a $REG_ADDR -f 4 -w $WARNING -c $CRITICAL)
    rc=$?
    
    # 0 - OK
    # 1 - WARNING
    # 2 - CRITICAL
    if [ $rc -gt 2 -o $rc -lt 0 ]; then
        echo $result
        exit $rc
    fi


    state=$( echo $result | awk -F ':' '{ print $1 }' )
    if [ $? -ne 0 ]; then
        echo "Failed to parse state"
        echo $result
        exit -10
    fi
    
    value=$( echo $result | awk -F ':' '{ print $2 }' )
    if [ $? -ne 0 ]; then
        echo "Failed to parse read value"
        echo $result
        exit -10
    fi    
}

print_results()
{
    # print converted results
    echo "$state:$value"
    exit $rc
}


#*********************************************************



# check command line parameters
if [ "$4" == "" ]; then
    echo "Help: $0 <IP address> <Modbus register> <Warning> <Critical>"
    exit -11
fi
check_software

#*********************************************************
#
# to support new format data just rewrite this section
#
#*********************************************************

# process command line  and convert input parameters
DIV_PNT=1000
IP=$1
REG_ADDR=$2
WARNING=$( echo $3*$DIV_PNT  | bc -l )
CRITICAL=$( echo $4*$DIV_PNT  | bc -l )
# print_settings

read_modbus

# convert output data 
value=$( echo $value / $DIV_PNT | bc -l )

#*********************************************************

print_results
