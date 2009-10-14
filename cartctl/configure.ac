#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.5])
AC_INIT([cartctl Library], [0.2.0], [cradelit@gmail.com], [cartctl])
AM_INIT_AUTOMAKE([cartctl], [0.2.0])
LT_INIT()
AC_CONFIG_SRCDIR([cartctl/nes.h])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for header files.
#AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h termio.h unistd.h getopt.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([bzero strcasecmp getopt_long malloc free])

AC_CONFIG_FILES([Makefile
                 cartctl/Makefile
		 ])

AC_OUTPUT
