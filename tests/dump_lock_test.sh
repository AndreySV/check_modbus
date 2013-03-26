#!/bin/bash

MODBUS_TCP_SERVER=192.168.56.1
DUMP_FILE=dump.bin

function run_create_dumpfile()
{
    while true; do

        cnt_cd=$(( $cnt_cd + 1 ))
        show_cd=$(( $cnt_cd % 50 ))
        if [ $show_cd -eq 0 ]; then
            echo "cnt_cd  = $cnt_cd"
        fi

        ./check_modbus --ip $MODBUS_TCP_SERVER   -f 3 --dump --dump_format=1 --dump_size=10 --dump_file=$DUMP_FILE 

        rc_create_dump=$?
        if [ $rc_create_dump -ne 0 ]; then
            echo "ERROR: in run_create_dumpfile( $rc_create_dump )"
        fi
        sleep $1

    done;
}



# as input get rc variable name
function run_check_dumpfile()
{
    addr=$1
    cnt[$addr]=0
    while true; do
        cnt[$addr]=$(( ${cnt[$addr]} + 1 ))
        show=$(( ${cnt[$addr]} % 100 ))
        if [ $show -eq 0 ]; then
            echo "cnt[ $addr ] = ${cnt[$addr]}"
        fi

        sleep $2
        ./check_modbus --file $DUMP_FILE -f 3 -a $addr > /dev/null 
        rc[$addr]=$?
        case ${rc[$addr]} in
            [0-2])
                continue
                ;;
            *)
                echo "ERROR: in run run_check_dumpfile $addr ( ${rc[$addr]} )"
                ;;
        esac
    done;
}


# run in background 
( run_check_dumpfile 1 0.5 ) &   
( run_check_dumpfile 2 2.8 ) &    
( run_check_dumpfile 3 1.8 ) & 

run_create_dumpfile 1




