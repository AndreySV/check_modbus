/*
  check_modbus: checker for Modbus TCP/RTU devices for Nagios control
  system based on libmodbus library

  Copyright (C) 2011 2012 2013 2014 Andrey Skvortsov


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


  Andrey Skvortsov
  Andrej . Skvortzov [at] gmail . com
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

#include "command_line.h"
#include "compile_date_time.h"
#include "global_macro.h"
#include "check_modbus.h"
#include "dbg_printf.h"

static void print_version(void)
{
	printf("Check ModBus version %s\n", PACKAGE_VERSION);
	printf("Build date: %02d.%02d.%04d\n", COMPILE_DAY, COMPILE_MONTH, COMPILE_YEAR);
	printf("\n");
}


static void print_help(void)
{
	print_version();
	printf("--version           Print version information\n");
	printf("-v  --verbose       Print additional (debug) information (settings, modbus debug etc).\n");
	printf("                    Specify multiple times to increase verbosity level.\n");
	printf("-h  --help          Print this help\n");
	printf("-H  --ip=           IP address or hostname\n");
	printf("-p  --port=         [ TCP Port number. Default %s ]\n", XSTR(MODBUS_TCP_DEFAULT_PORT));

#if LIBMODBUS_VERSION_MAJOR >= 3
	printf("-S  --serial=       Serial port to use\n");
	printf("-b  --serial_bps=   [ Serial port speed. Default 9600 ]\n");
	printf("    --serial_mode=  [ RS mode of serial port. Default 0 ]\n");
	printf("                            0 - RS232\n");
	printf("                            1 - RS485\n");
	printf("    --serial_parity=  [ Serial port parity settings. Default none ]\n");
	printf("                            Allowed values: none/N, even/E, odd/O\n");
	printf("    --serial_data_bits=  [ Serial port number of data bits. Default 8 ]\n");
	printf("                            Allowed values: 5, 6, 7, 8\n");
	printf("    --serial_stop_bits=  [ Serial port number of stop bits. Default 1 ]\n");
	printf("                            Allowed values: 1, 2\n");
#endif

	printf("--file=             use binary dump file as input source\n");

	printf("-d  --device=       [ Device modbus number. Default 1 ]\n");
	printf("-a  --address=      [ Register/bit address reference. Default 1 ]\n");
	printf("-t  --try=          [ Number of tries. Default 1 ]\n");
	printf("-F  --format=       [ Data format. Default 1 ]\n");
	printf("                        1 -  int16_t\n");
	printf("                        2 - uint16_t\n");
	printf("                        3 -  int32_t\n");
	printf("                        4 - uint32_t\n");
	printf("                        5 -  int64_t\n");
	printf("                        6 - uint64_t\n");
	printf("                        7 -  float\n");
	printf("                        8 -  double\n");
	printf("-s  --swapbytes     [ Swap bytes in each incomming word ]\n");
	printf("-i  --inverse       [ Use inversed words order ]\n");
	printf("-f  --function=     Number of functions\n");
	printf("                        1 - Read coils\n");
	printf("                        2 - Read input discretes\n");
	printf("                        3 - Read multiple registers\n");
	printf("                        4 - Read input registers\n");
	printf("-w  --warning=      [ Warning range ]\n");
	printf("-c  --critical=     [ Critical range ]\n");
	printf("-n  --null          [ If the query will get zero, return the critical signal ]\n");
	printf("-N  --not_null      [ If the query will get no zero, return the critical signal ]\n");
	printf("\n");
	printf("-m  --perf_min=     [ Minimum value for performance data]\n");
	printf("-M  --perf_max=     [ Maximum value for performance data]\n");
	printf("-P  --perf_data     [ Enable to show performance data. By default performance data is disabled]\n");
	printf("-L  --perf_label=   [ Label for performance data]\n");
	printf("\n");
	printf("--dump              [ Dump register and bits values instead of analize their values ]\n");
	printf("--dump_size=        [ Number of registers/bits in output dump ]\n");
	printf("--dump_format=      [ Format of dump output. Default 1]\n");
	printf("                        1 - binary\n");
	printf("                        2 - hex\n");
	printf("                        3 - decimal\n");
	printf("--dump_file=        [ Output dump file ]\n");
	printf("\n");
	printf("--lock_file_in =    [ Lock file for input operation (access to dump file or serial device ]\n");
	printf("--lock_file_out =   [ Lock file for output operations (for example to access dump file) ]\n");
	printf("\n");
	printf(" Examples:\n");
	printf("          ./check_modbus --ip=192.168.1.123 -d 1 -a 13 -f 4 -w 123.4 -c 234.5\n");
	printf("          ./check_modbus --ip=192.168.1.123 -d 1 -a 15 -f 4 -w 2345 -c 1234\n");
	printf("          ./check_modbus --ip=plc01 --try=5 -d 2 -a 20 -f 2 -n\n");
	printf("          ./check_modbus --ip=plc01 --try=5 -d 2 -a 1 -f 4 --dump --dump_format 1 --dump_size 20\n");
	printf("          ./check_modbus --file=file.dump -F 7 -f 4 -a 20 -w 100\n");

#if LIBMODBUS_VERSION_MAJOR >= 3
	printf("          ./check_modbus --serial=/dev/ttyS0 -d 2 -a 7 -f 4 -n\n");
#endif

	printf("\n");
}

void print_settings(FILE *fd, struct modbus_params_t *params)
{
	fprintf(fd, "---------------------------------------------\n");
	fprintf(fd, "Settings:\n");

	if (params->host != NULL) {
		fprintf(fd, "ip:          %s\n",          params->host);
		fprintf(fd, "port:        %s\n",          params->mport);
	}
#if LIBMODBUS_VERSION_MAJOR >= 3
	else if (params->serial != NULL) {
		fprintf(fd, "serial:           %s\n",     params->serial);
		fprintf(fd, "serial_mode:      %s\n",     (params->serial_mode == MODBUS_RTU_RS232) ? "MODBUS_RTU_RS232" : "MODBUS_RTU_RS485");
		fprintf(fd, "serial_bps:       %d\n",     params->serial_bps);
		fprintf(fd, "serial_parity:    %c (N: none, E: even, O: odd)\n",     params->serial_parity);
		fprintf(fd, "serial_data_bits: %d\n",     params->serial_data_bits);
		fprintf(fd, "serial_stop_bits: %d\n",     params->serial_stop_bits);
	}
#endif
	else if (params->file != NULL)
		fprintf(fd, "file:             %s\n",     params->file);

	fprintf(fd, "\n");

	fprintf(fd, "verbosity:   %d\n",          params->verbose);

	fprintf(fd, "device:      %d\n",          params->devnum);
	fprintf(fd, "address:     %d\n",          params->sad);
	fprintf(fd, "function:    %d\n",          params->nf);
	fprintf(fd, "tries:       %d\n",          params->tries);
	fprintf(fd, "\n");
	fprintf(fd, "inverse:     %d\n",          params->inverse_words);
	fprintf(fd, "format:      %d\n",          params->format);
	fprintf(fd, "swap bytes:  %d\n",          params->swap_bytes);
	fprintf(fd, "\n");

	fprintf(fd, "warning range:\n");
	fprint_range(fd, &params->warn_range);

	fprintf(fd, "critical range:\n");
	fprint_range(fd, &params->crit_range);

	fprintf(fd, "null:        %d\n",          params->nc);
	fprintf(fd, "not null:    %d\n",          params->nnc);
	fprintf(fd, "\n");
	fprintf(fd, "perf_data:   %d\n",          params->perf_data);


	fprintf(fd, "perf_label:  %s\n",          params->perf_label ? params->perf_label : "NULL");

	fprintf(fd, "perf_min:    %f\n",          params->perf_min);
	fprintf(fd, "perf_max:    %f\n",          params->perf_max);

	fprintf(fd, "\n");
	fprintf(fd, "dump:        %d\n",          params->dump);
	fprintf(fd, "dump_format: %d\n",          params->dump_format);
	fprintf(fd, "dump_size:   %d\n",          params->dump_size);
	fprintf(fd, "dump_file :  %s\n",          params->dump_file ? params->dump_file : "stdout");
	fprintf(fd, "\n");
	fprintf(fd, "lock_file_in :%s\n",         params->lock_file_in ? params->lock_file_in : "NULL");
	fprintf(fd, "lock_file_out:%s\n",         params->lock_file_out ? params->lock_file_out : "NULL");
	fprintf(fd, "\n");
	fprintf(fd, "gain:         %f\n",         params->gain);
	fprintf(fd, "offset:       %f\n",         params->offset);
	fprintf(fd, "---------------------------------------------\n");
}


static void    load_defaults(struct modbus_params_t *params)
{
	static char  mport_default[] = XSTR(MODBUS_TCP_DEFAULT_PORT);

#if LIBMODBUS_VERSION_MAJOR >= 3
	static char  serial_parity_default = SERIAL_PARITY_DEFAULT;

	params->serial           = NULL;
	params->serial_mode      = MODBUS_RTU_RS232;
	params->serial_bps       = 9600;
	params->serial_parity    = serial_parity_default;
	params->serial_data_bits = 8;
	params->serial_stop_bits = 1;
#endif
	params->mport       = mport_default;

	params->file        = NULL;

	params->sad         = 0;
	params->devnum      = 1;
	params->host        = NULL;
	params->nf          = 0;
	params->nc          = 0;
	params->nnc         = 0;
	params->tries       = 1;
	params->format      = 1;
	params->inverse_words  = 0;
	params->swap_bytes  = 0;

	params->warn_range.defined = 0;
	params->warn_range.lo = 0;
	params->warn_range.hi = 0;

	params->crit_range.defined = 0;
	params->crit_range.lo = 0;
	params->crit_range.hi = 0;

	params->verbose     = 0;

	params->perf_min_en = 0;
	params->perf_max_en = 0;
	params->perf_data   = 0;
	params->perf_label  = NULL;
	params->perf_min    = 0;
	params->perf_max    = 0;

	params->dump        = 0;
	params->dump_format = 0;
	params->dump_size   = 0;
	params->dump_file   = NULL;

	params->lock_file_in = NULL;
	params->lock_file_in_fd = 0;

	params->lock_file_out = NULL;
	params->lock_file_out_fd = 0;

	params->gain = 1.0;
	params->offset = 0.0;
}


static int check_swap_inverse(struct modbus_params_t *params)
{
	int rc = 0;

	if (((params->nf == 1) || (params->nf == 2)) && /* bit operations */
		((params->swap_bytes) || (params->inverse_words)))
		rc = 1;
	if (rc)	{
		ERR("Swap bytes and inverse words functionality not acceptable ");
		ERR("for modbus functions 1 and 2 operated with bits.\n");
	}
	return rc;
}

static int check_dump_param(struct modbus_params_t *params)
{
	int rc = 0;
	int ft = params->dump_format;

	rc =  (ft > DUMP_FMT_MIN_SUPPORTED) && (ft < DUMP_FMT_MAX_SUPPORTED) ? 0 : params->dump;

	return rc;
}

static int check_function_num(struct modbus_params_t *params)
{
	int rc;

	rc =  (params->nf > MBF_MIN_SUPPORTED) && (params->nf < MBF_MAX_SUPPORTED) ? 0 : 1;
	if (rc)
		ERR("Invalid function number: %d\n", params->nf);
	return rc;
}


static int check_source(struct modbus_params_t *params)
{
	int cnt;

	cnt = params->host   ? 1 : 0;
#if LIBMODBUS_VERSION_MAJOR >= 3
	if (params->serial)
		cnt++;
#endif
	if (params->file)
		cnt++;

	if (cnt > 1) {
		ERR("Several modbus input interfaces were declared\n");
		return 1;
	}
	return 0;
}


static int     check_format_type(struct modbus_params_t *params)
{
	int rc;
	int ft;
	int max_format, min_format;

	min_format = params->dump ? FORMAT_DUMP_MIN  : FORMAT_MIN_SUPPORTED;
	max_format = params->dump ? FORMAT_DUMP_MAX  : FORMAT_MAX_SUPPORTED;
	ft = params->format;

	rc =  (ft > min_format) && (ft < max_format) ? 0 : 1;
	if (rc)	{
		ERR("Invalid data format: %d\n", params->format);
		if (params->dump)
			ERR("-F (--format) parameter can not be used in dump mode\n");
	}
	return rc;
}


#if LIBMODBUS_VERSION_MAJOR >= 3
static int     check_serial_parity(char parity)
{
	return ((parity == 'N') || (parity == 'E') || (parity == 'O')) ? 0 : 1;
}
#endif




static int      check_command_line(struct modbus_params_t *params, int argc, char **argv)
{
	(void)argc;

#if LIBMODBUS_VERSION_MAJOR >= 3
	if (params->host == NULL && params->serial == NULL && params->file == NULL) {
		ERR("Not provided or unable to parse host address/serial port name/filename: %s\n",
			argv[0]);
		return RESULT_WRONG_ARG;
	};
#else
	if (params->host == NULL && params->file == NULL) {
		ERR("Not provided or unable to parse host address or filename: %s\n", argv[0]);
		return RESULT_WRONG_ARG;
	};
#endif

#if LIBMODBUS_VERSION_MAJOR >= 3
	if (params->serial != NULL) {
		if (params->serial_mode != MODBUS_RTU_RS232 && params->serial_mode != MODBUS_RTU_RS485)	{
			ERR("%s: Invalid value of serial port mode parameter!\n", argv[0]);
			return RESULT_WRONG_ARG;
		}
		if (check_serial_parity(params->serial_parity)) {
			ERR("%s: Invalid value of serial port parity mode parameter!\n", argv[0]);
			return RESULT_WRONG_ARG;
		}
		if (params->serial_data_bits < 5 || params->serial_data_bits > 8) {
			ERR("%s: Invalid value of serial port mode data length parameter!\n", argv[0]);
			return RESULT_WRONG_ARG;
		}
		if (params->serial_stop_bits < 1 || params->serial_stop_bits > 2) {
			ERR("%s: Invalid value of serial port stop bits parameter!\n", argv[0]);
			return RESULT_WRONG_ARG;
		}
	}
#endif
	if (params->perf_data && (params->perf_label == NULL)) {
		ERR("Label parameter is required, when performance data is enabled\n");
		return RESULT_WRONG_ARG;
	}

	if (params->dump_size > 127) {
		ERR("The maximal number of registers in one dump is 127\n");
		return RESULT_WRONG_ARG;
	}


	if (check_swap_inverse(params))
		return RESULT_WRONG_ARG;

	if (check_function_num(params))
		return RESULT_WRONG_ARG;

	if (check_format_type(params))
		return RESULT_WRONG_ARG;

	if (check_source(params))
		return RESULT_WRONG_ARG;

	if (check_dump_param(params))
		return RESULT_WRONG_ARG;

	if (dbg_chk_level(DBG_INFO))
		print_settings(stdout, params);

	return RESULT_OK;
}


	/* no short option char wasted for rarely used options */
	enum {
		OPT_LONG_OPTIONS_ONLY = 0x100,
#if LIBMODBUS_VERSION_MAJOR >= 3
		OPT_SERIAL_MODE,
		OPT_SERIAL_PARITY,
		OPT_SERIAL_DATA_BITS,
		OPT_SERIAL_STOP_BITS,
#endif
		OPT_FILE,

		OPT_DUMP,
		OPT_DUMP_FILE,
		OPT_DUMP_FORMAT,
		OPT_DUMP_SIZE,

		OPT_LOCK_FILE_IN,
		OPT_LOCK_FILE_OUT,

		OPT_GAIN,
		OPT_OFFSET,
		OPT_VERSION
	};

#if LIBMODBUS_VERSION_MAJOR >= 3
static	const char *short_options = "hH:p:S:b:d:a:f:w:c:nNt:F:isvPm:M:L:";
#else
static	const char *short_options = "hH:p:d:a:f:w:c:nNt:F:isvPm:M:L:";
#endif
static	const struct option long_options[] = {
	{"help",          no_argument,            NULL,  'h'   },
	{"ip",            required_argument,      NULL,  'H'   },
	{"port",          required_argument,      NULL,  'p'   },
#if LIBMODBUS_VERSION_MAJOR >= 3
	{"serial",        required_argument,      NULL,  'S'   },
	{"serial_mode",   required_argument,      NULL,  OPT_SERIAL_MODE},
	{"serial_bps",    required_argument,      NULL,  'b'   },
	{"serial_parity", required_argument,      NULL,  OPT_SERIAL_PARITY},
	{"serial_data_bits", required_argument,   NULL,  OPT_SERIAL_DATA_BITS},
	{"serial_stop_bits", required_argument,   NULL,  OPT_SERIAL_STOP_BITS},
#endif
	{"file",          required_argument,      NULL,  OPT_FILE},
	{"device",        required_argument,      NULL,  'd'   },
	{"address",       required_argument,      NULL,  'a'   },
	{"try",           required_argument,      NULL,  't'   },
	{"function",      required_argument,      NULL,  'f'   },
	{"format",        required_argument,      NULL,  'F'   },
	{"function",      required_argument,      NULL,  'f'   },
	{"critical",      required_argument,      NULL,  'c'   },
	{"null",          no_argument,            NULL,  'n'   },
	{"not_null",      no_argument,            NULL,  'N'   },
	{"swapbytes",     no_argument,            NULL,  's'   },
	{"inverse",       no_argument,            NULL,  'i'   },
	{"verbose",       no_argument,            NULL,  'v'   },
	{"perf_data",     no_argument,            NULL,  'P'   },
	{"perf_min",      required_argument,      NULL,  'm'   },
	{"perf_max",      required_argument,      NULL,  'M'   },
	{"perf_label",    required_argument,      NULL,  'L'   },
	{"dump",          no_argument,            NULL,   OPT_DUMP        },
	{"dump_size",     required_argument,      NULL,   OPT_DUMP_SIZE   },
	{"dump_format",   required_argument,      NULL,   OPT_DUMP_FORMAT },
	{"dump_file",     required_argument,      NULL,   OPT_DUMP_FILE   },
	{"lock_file_in",  required_argument,      NULL,   OPT_LOCK_FILE_IN   },
	{"lock_file_out", required_argument,      NULL,   OPT_LOCK_FILE_OUT  },
	{"gain",          required_argument,      NULL,   OPT_GAIN },
	{"offset",        required_argument,      NULL,   OPT_OFFSET },
	{"version",       no_argument,            NULL,   OPT_VERSION },
	{NULL,            0,                      NULL,   0    },
};

static int parse_int_param(char *arg, int *value)
{
	char *end;

	errno = 0;
	*value = strtoul(arg, &end, 0);
	if ((end && *end) || errno) {
		ERR("wrong parameter value %s\n", arg);
		return RESULT_WRONG_ARG;
	}
	return RESULT_OK;
}

static int parse_double_param(char *arg, double *value)
{
	char *end;

	errno = 0;
	*value = strtod(arg, &end);
	if ((end && *end) || errno) {
		ERR("wrong parameter value %s\n", arg);
		return RESULT_WRONG_ARG;
	}
	return RESULT_OK;
}


int     parse_command_line(struct modbus_params_t *params, int argc, char **argv)
{
	int rs;
	int rc = RESULT_OK;
	int option_index;

	if (argc < 2) {
		ERR("%s: Could not parse arguments\n", argv[0]);
		print_help();
		return RESULT_WRONG_ARG;
	};

	load_defaults(params);
	while (1) {
		rs = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (rs == -1)
			break;


		switch (rs) {
		case 'v':
			params->verbose++;
			dbg_set_level(params->verbose);
			break;
		case 'h':
			print_help();
			return RESULT_PRINT_HELP;

			/* MODBUS TCP */
		case 'H':
			params->host = optarg;
			break;
		case 'p':
			params->mport = optarg;
			break;

#if LIBMODBUS_VERSION_MAJOR >= 3
			/* MODBUS RTU */
		case 'S':
			params->serial = optarg;
			break;
		case OPT_SERIAL_MODE:
			rc = parse_int_param(optarg, &params->serial_mode);
			break;
		case 'b':
			rc = parse_int_param(optarg, &params->serial_bps);
			break;
		case OPT_SERIAL_PARITY:
			params->serial_parity = (optarg) ? toupper(*optarg) : '\0';
			break;
		case OPT_SERIAL_DATA_BITS:
			rc = parse_int_param(optarg, &params->serial_data_bits);
			break;
		case OPT_SERIAL_STOP_BITS:
			rc = parse_int_param(optarg, &params->serial_stop_bits);
			break;
#endif
		case OPT_FILE:
			params->file = optarg;
			break;
		case 'd':
			rc = parse_int_param(optarg, &params->devnum);
			break;
		case 'a':
			rc = parse_int_param(optarg, &params->sad);
			if (params->sad <= 0) {
				ERR("Address should be greater than zero: %s\n", optarg);
				rc = RESULT_WRONG_ARG;
			}
			params->sad--; /* register/bit address starts from 0 */
			break;
		case 'f':
			rc = parse_int_param(optarg, &params->nf);
			break;
		case 'F':
			rc = parse_int_param(optarg, &params->format);
			break;
		case 'w':
			if (parse_range(optarg, &params->warn_range) != 0) {
				ERR("can't parse warning range %s\n", optarg);
				rc = RESULT_WRONG_ARG;
			}
			break;
		case 'c':
			if (parse_range(optarg, &params->crit_range) != 0) {
				ERR("can't parse critical range %s\n", optarg);
				rc = RESULT_WRONG_ARG;
			}
			break;
		case 'n':
			params->nc = 1;
			break;
		case 'i':
			params->inverse_words = 1;
			break;
		case 's':
			params->swap_bytes = 1;
			break;
		case 't':
			rc = parse_int_param(optarg, &params->tries);
			break;
		case 'N':
			params->nnc = 1;
			break;
		case 'm':
			rc = parse_double_param(optarg, &params->perf_min);
			params->perf_min_en = 1;
			break;
		case 'M':
			rc = parse_double_param(optarg, &params->perf_max);
			params->perf_max_en = 1;
			break;
		case 'L':
			params->perf_label = optarg;
			break;
		case 'P':
			params->perf_data = 1;
			break;
		case OPT_DUMP_FILE:
			params->dump_file = optarg;
			break;
		case OPT_DUMP:
			params->dump = 1;
			break;
		case OPT_DUMP_SIZE:
			rc = parse_int_param(optarg, &params->dump_size);
			break;
		case OPT_DUMP_FORMAT:
			rc = parse_int_param(optarg, &params->dump_format);
			switch (params->dump_format) {
			case DUMP_FMT_BIN:
				params->format = FORMAT_DUMP_BIN;
				break;
			case DUMP_FMT_HEX:
				params->format = FORMAT_DUMP_HEX;
				break;
			case DUMP_FMT_DEC:
				params->format = FORMAT_DUMP_DEC;
				break;
			}
			break;
		case OPT_LOCK_FILE_IN:
			params->lock_file_in = optarg;
			break;
		case OPT_LOCK_FILE_OUT:
			params->lock_file_out = optarg;
			break;

		case OPT_GAIN:
			rc = parse_double_param(optarg, &params->gain);
			break;
		case OPT_OFFSET:
			rc = parse_double_param(optarg, &params->offset);
			break;
		case OPT_VERSION:
			print_version();
			rc = RESULT_PRINT_HELP;
			break;
		case '?':
		default:
			rc = RESULT_PRINT_HELP;
		};

		if (rc != RESULT_OK)
			return rc;
	};  /* while(1) */

	return check_command_line(params, argc, argv);
}
