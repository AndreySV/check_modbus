#ifndef _RANGES_H_
#define _RANGES_H_

struct range {
	const char   *src;
	double	      lo;
	double	      hi;
	unsigned int	inverted:1;
	unsigned int	defined:1;

};

int  parse_range(const char *spec, struct range *range);

int  check_ranges(
	const struct range *warning,
	const struct range *critical,
	const double value);

void fprint_range(FILE *file, const struct range *range);

#endif
