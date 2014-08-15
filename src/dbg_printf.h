#ifndef _DBG_PRINTF_H_
#define _DBG_PRINTF_H_

#include <stdbool.h>

enum {
	DBG_ERROR = 0,
	DBG_INFO,
	DBG_DEBUG
};


#define PRINT_LL(LEVEL, ...)						\
	do {								\
		dbg_printf(LEVEL, __FILE__, __LINE__, __func__,__VA_ARGS__); \
	} while (0)


#define INFO(...) PRINT_LL(DBG_INFO,  __VA_ARGS__)
#define DBG(...)  PRINT_LL(DBG_DEBUG, __VA_ARGS__)
#define ERR(...)  PRINT_LL(DBG_ERROR, __VA_ARGS__)


void dbg_set_level(int level);
int  dbg_get_level(void);
bool dbg_chk_level(int level);

void dbg_printf(int level,
		const char *file,
		unsigned int line,
		const char *function,
		const char *format,
		...);




#endif
