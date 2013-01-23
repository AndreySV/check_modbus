#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.67])
AC_INIT(check_modbus, 0.4, Skvortsov-at-kipspb.ru)
AM_INIT_AUTOMAKE(check_modbus,0.4)

AC_CONFIG_SRCDIR([src/check_modbus.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_SEARCH_LIBS([modbus_set_slave], [modbus], [], [
  AC_MSG_ERROR([unable to find the modbus library  ])
])


# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h modbus.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([modbus_new_tcp_pi],,AC_MSG_ERROR([echo Function modbus_new_tcp_pi is not found]))

# Output files
AC_CONFIG_FILES([Makefile src/Makefile ])

AC_OUTPUT
