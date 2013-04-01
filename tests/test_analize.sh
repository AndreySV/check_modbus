#!/bin/bash



#*********************************************************
# name of function for with testcase should have name 
#
#        test_[any string][_disable]
#
# tests with "_disable" string in the name
# will not be executed
#*********************************************************

function test_1
{
    check_modbus --file $DUMP_FILE_SRC --lock_file_out $LOCK_FILE  -f 3 -a 1 -N  >> $TMP_LOG_FILE 2>&1
    rc=$?
    rc_need=0
    if [ $rc -ne $rc_need ]; then
        echo "failed $FUNCNAME(): rc = $rc, but expected $rc_need" >> $TMP_LOG_FILE
        return 1
    fi
    return 0
}


function test_2
{
    check_modbus --file $DUMP_FILE_SRC --lock_file_out $LOCK_FILE  -f 3 -a 1 -n  >> $TMP_LOG_FILE 2>&1
    rc=$?
    rc_need=2
    if [ $rc -ne $rc_need ]; then
        echo "failed $FUNCNAME(): rc = $rc, but expected $rc_need" >> $TMP_LOG_FILE
        return 1
    fi
    return 0
}


function test_3
{
    params='-a 3 -w 2 -c 7';
    check_modbus --file $DUMP_FILE_SRC --lock_file_out $LOCK_FILE  -f 3 $params  >> $TMP_LOG_FILE 2>&1
    rc=$?
    rc_need=1
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
    
    # find all enabled test in the script
    all_tests=$( cat $( basename $0 ) | grep -P '^function[ \t]+test_' | grep -v '_disable' | sed 's/.*function[ \t]*//' )

    for test in $all_tests; do 
        
        echo > $TMP_LOG_FILE # clear log file
        echo '------------------------------------' >> $TMP_LOG_FILE
        echo $0 >> $TMP_LOG_FILE    
        eval $test
        if [ $? -ne 0 ]; then
            ret=1
            cat $TMP_LOG_FILE >> $LOG_FILE
        fi

    done;
    
    # finish testing

    rm $TMP_LOG_FILE >> /dev/null
    if [ $ret -ne 0 ]; then
        echo "Some tests failed. For additional information see $LOCK_FILE"
    else
        rm  $LOG_FILE >> /dev/null
    fi
    return $ret
}





#******************************************************************************
# settings
#******************************************************************************

TEST_NAME=$( basename $0 .sh)

# path to compiled binary
PATH=../src/:$PATH  


# VERBOSE=1
DUMP_FILE_SRC=${TEST_NAME}_dump.bin
LOCK_FILE=${TEST_NAME}.lock
LOG_FILE=${TEST_NAME}.log
TMP_LOG_FILE=${TEST_NAME}_tmp.log

run_all_test
exit $?
