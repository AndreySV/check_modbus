#ifndef _CHECK_MODBUS_H_
#define _CHECK_MODBUS_H_

#include "variant.h"

enum
{
    RESULT_OK   =   0,
    RESULT_WARNING,
    RESULT_CRITICAL,
    RESULT_ERROR,
    RESULT_UNKNOWN,
    RESULT_PRINT_HELP,
    RESULT_WRONG_ARG,
    RESULT_ERROR_CONNECT,
    RESULT_ERROR_READ,
    RESULT_UNSUPPORTED_FUNCTION,
    RESULT_UNSUPPORTED_FORMAT
};

typedef struct
{
    char*   mport; 		                // Port number
    int     devnum;		                // Device modbus address 
    int     sad;		                // register/bit address
    int     nf;			                // Number of function
    double  warn_range;		            // Warning range
    double  crit_range;		            // Critical range
    char    *host; 		                // IP address or host name
    int     nc;			                // Null flag
    int     nnc;		                // No null flag
    int     tries;		                // tries 
    int     format;		                // data format 
    int     swap_bytes;		            // bytes order
    int     inverse_words;		        // words order
    int     verbose;		            // verbose

    int     perf_min_en;
    int     perf_max_en;

    double  perf_min;                   // min value for performance data
    double  perf_max;                   // max value for performance data
    int     perf_data;		            // enable performance data
    char*   perf_label;                 // label for performance data
}   modbus_params_t;



enum  
{
    MBF_MIN_SUPPORTED= 0,
	MBF001_READ_COIL_STATUS,            // 0x01 <- STD CODES
	MBF002_READ_INPUT_STATUS,           // 0x02
	MBF003_READ_HOLDING_REGISTERS,      // 0x03
	MBF004_READ_INPUT_REGISTERS,        // 0x04
    MBF_MAX_SUPPORTED
};


#endif

