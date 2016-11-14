dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
dnl AC_PREREQ(2.59)
AC_INIT(check_modbus, 0.47, Andrej.Skvortzov-at-gmail.com)
AM_INIT_AUTOMAKE(check_modbus,0.47)

AC_CONFIG_SRCDIR([src/check_modbus.c])
AC_CONFIG_MACRO_DIR([m4])



dnl ---------------------------------------------
dnl Made possible to build for another arch.
dnl ---------------------------------------------

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

                


dnl Checks for programs.
AC_PROG_CC
AC_LANG_C
AC_PROG_MAKE_SET

dnl Checks for libraries.
AC_SEARCH_LIBS([modbus_set_slave], [modbus], [], [
  AC_MSG_ERROR([unable to find the modbus library  ])
])



dnl Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h modbus.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

dnl Checks for library functions.
AC_CHECK_FUNCS([modbus_new_tcp_pi],,AC_MSG_ERROR([echo Function modbus_new_tcp_pi is not found]))

dnl CC_CHECK_WERROR
CC_CHECK_CFLAGS_APPEND([ -Wall -Wextra -Wpedantic -Wc++-compat -Wmissing-declarations -Wmissing-prototypes])
CC_CHECK_CFLAGS_APPEND([ -Wformat-security -Wformat=2 -Wshadow -Wstrict-prototypes ])
CC_CHECK_CFLAGS_APPEND([ -Wmissing-include-dirs -Wbad-function-cast -Wwrite-strings -Wlogical-op ])
CC_CHECK_CFLAGS_APPEND([ -std=c99 -D_GNU_SOURCE ])
dnl CC_CHECK_CFLAGS_APPEND([ -Wconversion ])

AC_ARG_ENABLE(strict-build, AS_HELP_STRING([--enable-strict-build], [force warnings to be an error using gcc ]), [
                            CC_CHECK_CFLAGS_APPEND([ -Werror -Wfatal-errors]) ] ,
                            [])

dnl Output files
AC_CONFIG_FILES([
Makefile src/Makefile
tests/Makefile
m4/Makefile
])

AC_OUTPUT


