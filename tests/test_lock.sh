#!/bin/bash


#******************************************************************************
# run_create_dumpfile
#   
#      $1 - timeout between reading in seconds (can be float)
#      $2 - number of reading (by default nil that means 'never stop')
# 
#******************************************************************************
function run_create_dumpfile()
{
    local delay=$1
    local max_cnt=$2
    rm -f $QUIT_FLAG > /dev/null 2>&1
    rm -f $DUMP_FILE_DST > /dev/null 2>&1
    echo > $LOG_FILE # clear log file

    while true; do

        cnt_cd=$(( ${cnt_cd:=0} + 1 ))

        if [ "$VERBOSE" != "" ]; then
            show_cd=$(( $cnt_cd % 50 ))
            if [ $show_cd -eq 0 ]; then
                echo "cnt_cd  = $cnt_cd"
            fi
        fi

        if [ "$max_cnt" != "" ]; then
            if [ $cnt_cd -gt $max_cnt ]; then break; fi
        fi
        

        # check_modbus --ip $MODBUS_TCP_SERVER   -f 3 --dump --dump_format=1 --dump_size=10 --dump_file=$DUMP_FILE_DST
        check_modbus --file $DUMP_FILE_SRC --lock_file_out $LOCK_FILE   -f 3 --dump --dump_format=1 --dump_size=100 --dump_file=$DUMP_FILE_DST >> $LOG_FILE 2>&1

        local rc_create_dump=$?
        if [ $rc_create_dump -ne 0 ]; then
            echo "ERROR: in run_create_dumpfile( $rc_create_dump )" >> $LOG_FILE
            err_create_dump=$(( ${err_create_dump:=0} + 1 ))
        fi
        sleep $delay

    done;
    touch $QUIT_FLAG
}


#******************************************************************************
#
#  run_check_dumpfile $1 $2 $3
#
#      $1 - address of holding register 
#      $2 - timeout between reading in seconds (can be float)
#      $3 - number of reading (by default nil that means 'never stop')
# 
#******************************************************************************
function run_check_dumpfile()
{
    local addr=$1
    local delay=$2
    local max_cnt=$3

    cnt[$addr]=0
    while true; do
        sleep $delay

        cnt[$addr]=$(( ${cnt[$addr]} + 1 ))
        
        if [ "$VERBOSE" != "" ]; then
            local show=$(( ${cnt[$addr]} % 100 ))
            if [ $show -eq 0 ]; then
            echo "cnt[ $addr ] = ${cnt[$addr]}"
            fi
        fi

        if [ "$max_cnt" != "" ]; then
            if [ ${cnt[$addr]} -gt $max_cnt ]; then break; fi
        fi
        
        if [ -f $QUIT_FLAG ]; then
            # end the process and return error counter to the control process
            exit ${err_cnt[$addr]}
        fi

        local params
        local rc_need
        if [ "$(( $addr % 2 ))" -eq "0" ]; then
            # even register address
            params="-w 4368 -c 4370"
            rc_need=1
        else
            params="-N"
            rc_need=0
        fi


        check_modbus --file $DUMP_FILE_DST --lock_file_in $LOCK_FILE -f 3 -a $addr $params 2>> $LOG_FILE >/dev/null
        rc[$addr]=$?
        case ${rc[$addr]} in
            $rc_need)
                continue
                ;;
            *)
                echo "ERROR: in run run_check_dumpfile $addr ( ${rc[$addr]}, but need $rc_need )" >> $LOG_FILE
                err_cnt[$addr]=$(( ${err_cnt[$addr]:=0} + 1 ))
                ;;
        esac
    done;
}


function kill_background_jobs()
{
    my_pid=$$
    pids=$(ps -A | grep $( basename $0 ) | grep -v $my_pid | awk '{print $1}')
    kill -9 $pids > /dev/null 2>&1
    rm $QUIT_FLAG > /dev/null 2>&1
}


function get_clients_results()
{
    for i in $(seq $NUMBER_OF_CLIENT_STREAMS); do
        wait ${client_pid[$i]}
        client_errs=$(( ${client_errs:=0} + $?  ))        
    done;
    sleep 5
    kill_background_jobs
}




#******************************************************************************
# settings

# path to compiled binary
PATH=../src/:$PATH  

TEST_NAME=$( basename $0 .sh)


# VERBOSE=1
# MODBUS_TCP_SERnVER=192.168.56.1
DUMP_FILE_SRC=${TEST_NAME}_dump_src.bin
DUMP_FILE_DST=${TEST_NAME}_dump.bin
QUIT_FLAG=${TEST_NAME}_quit.tmp
LOCK_FILE=${TEST_NAME}.lock
LOG_FILE=${TEST_NAME}.log

NUMBER_OF_TESTS=900
NUMBER_OF_CLIENT_STREAMS=40


# run in background clients
for i in $( seq $NUMBER_OF_CLIENT_STREAMS ); do
    ( run_check_dumpfile $i 0.1 ) &
    client_pid[$i]=$!
done;


# 
run_create_dumpfile 0.1 $NUMBER_OF_TESTS

get_clients_results

# print results
echo 
echo $NUMBER_OF_TESTS times dump file was created
echo ${client_errs:=0} errors were by reading a dump file
echo ${err_create_dump:=0} errors were by creating a dump file
echo


if [[ ( $client_errs = 0 ) && ( $err_create_dump = 0 ) ]]; then 
    ret=0
    # clear

    rm -f $DUMP_FILE_DST
    rm -f $QUIT_FLAG
    rm -f $LOCK_FILE
    rm -f $LOG_FILE
else
    ret=1
fi

exit $ret





