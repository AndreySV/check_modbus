/*-
 * Copyright (c) 2007-2011 Varnish Software AS
 * All rights reserved.
 *
 * Author: Cecilie Fritzvold <cecilihf@linpro.no>
 * Author: Tollef Fog Heen <tfheen@varnish-software.com>
 * Author: Andrey Skvortsov <andrej.skvortzov@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 *
 * Nagios plugin for Varnish
 */

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <locale.h>
#include <assert.h>
#include <float.h>

#include "check_modbus.h"
#include "ranges.h"

/*
 * Parse a range specification
 */
int parse_range(const char *spec, struct range *range)
{
	const char *delim;
	char *end;

	range->src = spec;
	/* @ means invert the range */
	if (*spec == '@') {
		++spec;
		range->inverted = 1;
	} else {
		range->inverted = 0;
	}

	/* empty spec... */
	if (*spec == '\0')
		return (-1);

	if ((delim = strchr(spec, ':')) != NULL) {
		/*
		 * The Nagios plugin documentation says nothing about how
		 * to interpret ":N", so we disallow it.  Allowed forms
		 * are "~:N", "~:", "M:" and "M:N".
		 */
		if (delim - spec == 1 && *spec == '~') {
			range->lo = -DBL_MAX;
		} else {
			range->lo = strtod(spec, &end);
			if (end != delim)
				return (-1);
		}
		if (*(delim + 1) != '\0') {
			range->hi = strtod(delim + 1, &end);
			if (*end != '\0')
				return (-1);
		} else {
			range->hi = DBL_MAX;
		}
	} else {
		/*
		 * Allowed forms are N
		 */
		range->lo = 0;
		range->hi = strtod(spec, &end);
		if (*end != '\0')
			return (-1);
	}

	/*
	 * Sanity
	 */
	if (range->lo > range->hi)
		return (-1);

	range->defined = 1;
	return (0);
}

/*
 * Check if a given value is within a given range.
 */
static int inside_range(const double value, const struct range *range)
{
	if (range->inverted)
		return (value < range->lo || value > range->hi);
	return (value >= range->lo && value <= range->hi);
}


/*
 * Check if the thresholds against the value and return the appropriate
 * status code.
 */
int check_ranges(
	const struct range *warning,
	const struct range *critical,
	const double value)
{

	if (!warning->defined && !critical->defined)
		return (RESULT_UNKNOWN);
	if (critical->defined && !inside_range(value, critical))
		return (RESULT_CRITICAL);
	if (warning->defined && !inside_range(value, warning))
		return (RESULT_WARNING);
	return (RESULT_OK);
}

void fprint_range(FILE *file, const struct range *range)
{
	if (!range->defined) 
		fprintf(file, "\trange undefined\n");
	else {
		fprintf(file, "\tlow:      %f\n", range->lo);
		fprintf(file, "\thigh:     %f\n", range->hi);
		fprintf(file, "\tinverted: %d\n", range->inverted);
	}
}
