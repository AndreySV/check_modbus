/* Original version 0.01 made by Roman Suchkov aka Fineson
 * 
 * 0.31 version created by Andrey Skvortsov 
 */



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "modbus/modbus.h"
#include <errno.h>
#include "compile_date_time.h"
#include "command_line.h"





int     read_data(modbus_t* mb, modbus_params_t* params, data_t*    data)
{
    int rc;
    int size = sizeof_data_t( data );
    int sad  = params->sad;

    clear_data_t( data );
    switch (params->nf)
    {
        case MBF001_READ_COIL_STATUS: 
        	rc = modbus_read_bits(mb, sad, size , data->val.bytes);
            rc =  ((rc == -1) || (rc!=size)) ? RESULT_ERROR_READ : RESULT_OK;
            break;
        case MBF002_READ_INPUT_STATUS: 
            rc = modbus_read_input_bits(mb, sad, size, data->val.bytes);
            rc =  ((rc == -1) || (rc!=size)) ? RESULT_ERROR_READ : RESULT_OK;
            break;
        case MBF003_READ_HOLDING_REGISTERS: 
            rc = modbus_read_registers(mb, sad, size, data->val.words);
            rc =  ((rc == -1) || (rc!=size)) ? RESULT_ERROR_READ : RESULT_OK;
            break;
        case MBF004_READ_INPUT_REGISTERS: 
            rc = modbus_read_input_registers(mb, sad, size, data->val.words);
            rc =  ((rc == -1) || (rc!=size)) ? RESULT_ERROR_READ : RESULT_OK;
            break;
        default:
            rc = RESULT_UNSUPPORTED_FUNCTION;
            break;
    }
    reorder_data_t( data, params->swap_bytes, params->inverse_words );
    return rc;
}





int     print_error( int rc )
{
    switch( rc )
    {
        case RESULT_ERROR_CONNECT: 
            printf("Connection failed: %s\n:", modbus_strerror(errno) );
            break;
        case RESULT_ERROR_READ:
    	    printf("Read failed: %s\n", modbus_strerror(errno) );
            break;
        case RESULT_UNSUPPORTED_FORMAT:
            printf("Invalid data format\n");
            break;
        case RESULT_UNSUPPORTED_FUNCTION:
            printf("Invalid function number\n");
            break;
        case RESULT_OK:
            break;
        default:
            printf("Unsupported return code (%d)\n", rc);
            break;
    }
}

void print_performance_data(modbus_params_t* params, data_t* data)
{
    if (params->perf_data)
    {
        printf("\t\t|'%s'=", params->perf_label);
        printf_data_t( data );
        printf(";%lf;%lf;", params->warn_range, params->crit_range);
        if (params->perf_min_en) printf("%lf", params->perf_min );
        printf(";");
        if (params->perf_max_en) printf("%lf", params->perf_max );
    }
}

int print_result(modbus_params_t* params, data_t* data)
{
    int rc = RESULT_UNKNOWN;
    double   result, warn_range, crit_range;


    result      = value_data_t(data);
    warn_range  = params->warn_range;
    crit_range  = params->crit_range;

    if (params->nc != params->nnc )  
    {
        if (params->nc  == 1 )  rc = ( result == 0 ) ? RESULT_CRITICAL : RESULT_OK; 
        if (params->nnc == 1 )  rc = ( result != 0 ) ? RESULT_CRITICAL : RESULT_OK; 
    }
    else
    {
        if ( warn_range <= crit_range) 
        {
            if ( result >= crit_range)      rc = RESULT_CRITICAL;
            else rc = ( result >= warn_range) ? RESULT_WARNING : RESULT_OK;
        }
        else
        {
            if ( result <= crit_range)      rc = RESULT_CRITICAL;
            else rc = ( result <= warn_range) ?  RESULT_WARNING : RESULT_OK;
        }
    }


    switch(rc)
    {
        case RESULT_OK:
            printf("Ok: ");
            break;
        case RESULT_WARNING:
            printf("Warning: ");
            break;
        case RESULT_CRITICAL:
            printf("Critical: ");
            break;
        case RESULT_UNKNOWN:
            printf("Unknown result");
            break;
    }

    printf_data_t( data ); 
    print_performance_data( params, data );

    printf("\n"); 

    return rc;
}


int     process(modbus_params_t* params )
{
        modbus_t        *mb;
        const int       retry_max_timeout_us    =   100*1000; /* us */ 
        int             retry_timeout_us;
        struct timeval  response_timeout;
        int             try_cnt;
        data_t          data;
        int             rc;

        if (params->host != NULL) 
        {
            mb = modbus_new_tcp_pi(params->host, params->mport);
        }
#if LIBMODBUS_VERSION_MAJOR >= 3
        else if (params->serial != NULL) 
        {
            mb = modbus_new_rtu(params->serial, params->serial_bps, params->serial_parity, params->serial_data_bits, params->serial_stop_bits);
            if (mb != NULL) 
            {
                if (modbus_rtu_get_serial_mode(mb) != params->serial_mode) 
                {
                    rc = modbus_rtu_set_serial_mode(mb, params->serial_mode);
                    if (rc == -1) 
                    {
                        printf("Unable to set serial mode - %s (%d)\n", modbus_strerror(errno), errno);
                        return RESULT_ERROR;
                    }
                }
                else 
                {
                    if (params->verbose) printf("Serial port already in requested mode.\n");
                }    
            }
        }
#endif
        else 
        {
#if LIBMODBUS_VERSION_MAJOR >= 3
            printf( "Unable to allocate libmodbus context - unknown connection parameters: neither host nor serial port specified.\n");
#else
            printf( "Unable to allocate libmodbus context - unknown connection parameters.\n");
#endif
            return RESULT_ERROR;
        }

        if (params->verbose > 1) modbus_set_debug(mb, 1);
        
        if (mb == NULL) 
        {
            printf( "Unable to allocate libmodbus context\n");
            return RESULT_ERROR;
        }

        /* set short timeout */
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 0;
        modbus_set_response_timeout( mb, &response_timeout );

        /* calculate retry timeout : random from 1.0 to 1.3 of base timeout */
        retry_timeout_us = (1 + (rand() % 30)/100.0 )* ( retry_max_timeout_us*(try_cnt+1) );
        
        modbus_set_slave(mb,params->devnum);

        init_data_t( &data, params->format );
        for(try_cnt=0; try_cnt<params->tries; try_cnt++)
        {
            /* start new try */
            rc = RESULT_OK;

            if (modbus_connect(mb) == -1) 
            {
                usleep( retry_timeout_us );
                rc = RESULT_ERROR_CONNECT;
                continue;
            }

            rc = read_data( mb, params, &data);
            if (rc != RESULT_OK )
            {
	            modbus_close(mb);

                usleep( retry_timeout_us );
                continue;
            }
            else break;

        }

        print_error( rc ); 

        if (rc==RESULT_OK) rc = print_result( params, &data );
        
	    modbus_close(mb);
    	modbus_free(mb);
        return rc;
}




int main(int argc, char **argv)

{
    modbus_params_t  params;
    int                     rc;

    srand( time(NULL) );
    rc = parse_command_line(&params, argc, argv );
    return  ( rc != RESULT_OK ) ? rc : process( &params );
}

