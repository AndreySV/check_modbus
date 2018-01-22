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


#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/file.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "lock.h"
#include "compile_date_time.h"
#include "command_line.h"
#include "dbg_printf.h"



static int     read_data(modbus_t *mb, FILE *f, struct modbus_params_t *params, struct data_t *data)
{
	int rc = RESULT_OK;
	int size = sizeof_data_t(data);
	int sad  = params->sad;

	INFO("");

	clear_data_t(data);

	/* no source is specified */
	if ((!mb) && (!f))
		return RESULT_ERROR;

	if (mb != NULL)	{
		switch (params->nf) {
		case MBF001_READ_COIL_STATUS:
			rc = modbus_read_bits(mb, sad, size , data->val.bytes);
			rc =  ((rc == -1) || (rc != size)) ? RESULT_ERROR_READ : RESULT_OK;
			break;
		case MBF002_READ_INPUT_STATUS:
			rc = modbus_read_input_bits(mb, sad, size, data->val.bytes);
			rc =  ((rc == -1) || (rc != size)) ? RESULT_ERROR_READ : RESULT_OK;
			break;
		case MBF003_READ_HOLDING_REGISTERS:
			rc = modbus_read_registers(mb, sad, size, data->val.words);
			rc =  ((rc == -1) || (rc != size)) ? RESULT_ERROR_READ : RESULT_OK;
			break;
		case MBF004_READ_INPUT_REGISTERS:
			rc = modbus_read_input_registers(mb, sad, size, data->val.words);
			rc =  ((rc == -1) || (rc != size)) ? RESULT_ERROR_READ : RESULT_OK;
			break;
		default:
			rc = RESULT_UNSUPPORTED_FUNCTION;
			break;
		}
	}


	if (f != NULL) {
		size_t read_bytes;
		size_t need_bytes;

		if (fseek(f, sad*sizeof(data->val.words[0]), SEEK_SET))	{
			if (ferror(f))
				ERR("Error: %d error string: %s\n", errno, strerror(errno));
			ERR("Can not seek in file\n");
			return RESULT_ERROR;
		}

		need_bytes = sizeof(data->val.words[0])*size;
		read_bytes = fread(data->val.words, 1, need_bytes, f);

		if (read_bytes != need_bytes)	{
			if (ferror(f))
				ERR("Error: %d, error string: %s\n", errno, strerror(errno));
			if (feof(f))
				ERR("Error: end of file\n");
			ERR("Read only %ld bytes from file, but need %zu\n", read_bytes, need_bytes);
			return RESULT_ERROR_READ;
		}
		rc = RESULT_OK;
	}
	release_lock(params, LOCK_INPUT);

	if (rc == RESULT_OK)
		reorder_data_t(data, params->swap_bytes, params->inverse_words);

	INFO("rc: %d", rc);
	return rc;
}





static void     print_error(int rc)
{
	switch (rc) {
	case RESULT_ERROR_CONNECT:
		ERR("Connection failed: %s\n:", modbus_strerror(errno));
		break;
	case RESULT_ERROR_READ:
		ERR("Read failed: %s\n", modbus_strerror(errno));
		break;
	case RESULT_UNSUPPORTED_FORMAT:
		ERR("Invalid data format\n");
		break;
	case RESULT_UNSUPPORTED_FUNCTION:
		ERR("Invalid function number\n");
		break;
	case RESULT_OK:
		break;
	default:
		ERR("Unsupported return code (%d)\n", rc);
		break;
	}
}

static void print_performance_data(struct modbus_params_t *params, struct data_t *data)
{
	if (params->perf_data) {
		printf("\t\t|'%s'=", params->perf_label);
		printf_data_t(stdout, data);

		printf(";");
		if (params->warn_range.defined)
			printf("%s", params->warn_range.src);

		printf(";");
		if (params->crit_range.defined)
			printf("%s", params->crit_range.src);

		printf(";");
		if (params->perf_min_en)
			printf("%f", params->perf_min);

		printf(";");
		if (params->perf_max_en)
			printf("%f", params->perf_max);
	}
}

static void adjust_result(struct modbus_params_t *params, struct data_t *data)
{
	double value;

	if ((params->gain == 1.0) && (params->offset == 0.0))
		return;

	/* if adjust parameters are used change data type to double */
	value = value_data_t(data);
	value = params->gain*value + params->offset;
	init_data_t(data, FORMAT_DOUBLE, 0);
	data->val.long_real = value;
}



static const char * const status_text[] = {
	[RESULT_OK] = "OK",
	[RESULT_WARNING] = "WARNING",
	[RESULT_CRITICAL] = "CRITICAL",
	[RESULT_UNKNOWN] = "UNKNOWN",
};


static int print_result(struct modbus_params_t *params, struct data_t *data)
{
	int rc = RESULT_UNKNOWN;
	double   result;

	adjust_result(params, data);
	result      = value_data_t(data);

	INFO("%f", result);

	if (params->nc != params->nnc) {
		if (params->nc  == 1)
			rc = (result == 0) ? RESULT_CRITICAL : RESULT_OK;
		if (params->nnc == 1)
			rc = (result != 0) ? RESULT_CRITICAL : RESULT_OK;
	} else
		rc = check_ranges(&params->warn_range, &params->crit_range, result);


	printf("%s: ", status_text[rc]);
	INFO("rc: %d", rc);

	printf_data_t(stdout, data);
	printf(" %s", params->unit);
	print_performance_data(params, data);

	printf("\n");

	return rc;
}

static int init_connection(struct modbus_params_t *params, modbus_t **mb, FILE **f)
{
	int rc;
	struct timeval  response_timeout;

	*mb = NULL;
	*f  = NULL;

	rc = RESULT_OK;
	INFO("");

	set_lock(params, LOCK_INPUT);
	/*******************************************************************/
	/*                       Modbus-TCP                                */
	/*******************************************************************/
	if (params->host != NULL) {
		*mb = modbus_new_tcp_pi(params->host, params->mport);
		if (*mb == NULL) {
			ERR("Unable to allocate libmodbus context\n");
			return RESULT_ERROR;
		}
	}

	/*******************************************************************/
	/*                       Modbus-RTU                                */
	/*******************************************************************/
	if (params->serial != NULL) {
		*mb = modbus_new_rtu(params->serial, params->serial_bps, params->serial_parity, params->serial_data_bits, params->serial_stop_bits);
		if (*mb == NULL) {
			ERR("Unable to allocate libmodbus context\n");
			return RESULT_ERROR;
		}
	}
	/*******************************************************************/
	/*                       File input                                */
	/*******************************************************************/
	if (params->file != NULL) {

		*f = fopen(params->file, "rb");
		if (*f == NULL) {
			ERR("Unable to open binary dump file %s (%s)\n",
				params->file, strerror(errno));
			return RESULT_ERROR;
		}
	}


	/*******************************************************************/
	if (*mb != NULL) {
		if (dbg_chk_level(DBG_DEBUG))
			modbus_set_debug(*mb, 1);

		/* set short timeout */
		response_timeout.tv_sec = 1;
		response_timeout.tv_usec = 0;
#if (LIBMODBUS_VERSION_CHECK(3, 1, 2))
		modbus_set_response_timeout(*mb, response_timeout.tv_sec, response_timeout.tv_usec);
#else
		modbus_set_response_timeout(*mb, &response_timeout);
#endif
		modbus_set_slave(*mb, params->devnum);
	}

	INFO("rc: %d", rc);
	return rc;
}

static void     sleep_between_tries(int retry)
{
	const int       retry_max_timeout_us    =   100*1000; /* us */
	int             retry_timeout_us;

	/* calculate retry timeout : random from 1.0 to 1.3 of base timeout */
	retry_timeout_us = (1 + (rand() % 30)/100.0) * (retry_max_timeout_us * (retry+1));

	usleep(retry_timeout_us);
}

static void    deinit_connection(modbus_t **mb, FILE **f)
{
	if (*mb != NULL) {
		modbus_close(*mb);
		modbus_free(*mb);
		*mb = NULL;
	}
	if (*f != NULL) {
		fclose(*f);
		*f = NULL;
	}
}

static int     set_serial_mode(modbus_t *mb, struct modbus_params_t *params)
{
  int rc;
  if (modbus_rtu_get_serial_mode(mb) != params->serial_mode) {
    rc = modbus_rtu_set_serial_mode(mb, params->serial_mode);
    if (rc == -1) {
      ERR("Unable to set serial mode - %s (%d)\n", modbus_strerror(errno), errno);
      return RESULT_ERROR;
    }
  } else {
    if (dbg_chk_level(DBG_INFO))
      printf("Serial port already in requested mode.\n");
  }
  return RESULT_OK;
}

static int     open_modbus_connection(modbus_t *mb, struct modbus_params_t *params)
{
	int rc = RESULT_OK;

	if (mb != NULL) {
		if (modbus_connect(mb) == -1)
			rc = RESULT_ERROR_CONNECT;
		else {
#if LIBMODBUS_SERIAL_SUPPORTED
		  if (params->serial != NULL)
		    rc = set_serial_mode(mb, params);
#endif
		  /* flush old data from buffer */
		  modbus_flush(mb);
		}
	}

	return rc;
}


static void close_modbus_connection(modbus_t *mb)
{
	if (mb != NULL)
		modbus_close(mb);
}


static int     save_dump_file(struct modbus_params_t *params, struct data_t *data)
{

	/* Set exclusive lock for stdout. It's needed in dump mode to be
	   sure that noone can access unfinished file. Because
	   the dump file is created using stdout.  */

	FILE *fout;
	int   rc;

	set_lock(params, LOCK_OUTPUT);

	if (params->dump_file) {

		fout = fopen(params->dump_file, "wb");
		if (!fout) {
			ERR("Can't create file %s\n", params->dump_file);
			return RESULT_ERROR;
		}
	} else
		fout = stdout;

	printf_data_t(fout, data);
	rc = RESULT_OK;

	fclose(fout);

	release_lock(params, LOCK_OUTPUT);
	return rc;
}




static int process(struct modbus_params_t *params)
{
	modbus_t        *mb;
	FILE            *f;
	int             try_cnt;
	struct data_t   data;
	int             rc;



	rc = init_connection(params, &mb, &f);
	if (rc)
		return rc;


	init_data_t(&data, params->format, params->dump_size);
	for (try_cnt = 0; try_cnt < params->tries; try_cnt++) {
		/* start new try */
	        rc = open_modbus_connection(mb, params);
		if (rc == RESULT_OK) {
			rc = read_data(mb, f, params, &data);
			if (rc == RESULT_OK)
				break;

			close_modbus_connection(mb);
		}
		sleep_between_tries(try_cnt);
	}

	print_error(rc);

	if (rc == RESULT_OK) {
		if (params->dump)
			rc = save_dump_file(params, &data);
		else
			rc = print_result(params, &data);
	}

	deinit_connection(&mb, &f);
	INFO("rc: %d", rc);
	return rc;
}




int main(int argc, char **argv)

{
	int              rc;
	struct modbus_params_t  params;

	srand(time(NULL));
	rc = parse_command_line(&params, argc, argv);
	return  (rc != RESULT_OK) ? rc : process(&params);
}
