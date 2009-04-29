#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([copynesl], [0.3.6], [cradelit@gmail.com])
AM_INIT_AUTOMAKE([copynesl], [0.3.6])
AC_CONFIG_SRCDIR([copynesl])
AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH(inspect,
	AS_HELP_STRING([--with-inspect],[Inspect test suite errors]),
	[AC_DEFINE(INSPECT_ERRORS, 1, "inspect errors")],
	[AC_DEFINE(INSPECT_ERRORS, 0, "inspect errors")])

# Check for lincopynes
AC_MSG_CHECKING(for libcopynes)

AC_ARG_WITH(copynes,
    AC_HELP_STRING([--with-copynes@<:@=DIR@:>@],[Include copynes hardware support (default yes)]),
    [WITH_COPYNES=$withval],[WITH_COPYNES="yes"])
	
if test "$WITH_COPYNES" != "no"; then
	use_copynes=yes
	if test "x$WITH_COPYNES" = "xyes"; then
#default search location for libcopynes
		CFLAGS="$CFLAGS -I/usr/local/include/copynes"
		LDFLAGS="$LDFLAGS -L/usr/local/lib/copynes"
	else
		CFLAGS="$CFLAGS -I$WITH_COPYNES/include"
		LDFLAGS="$LDFLAGS -L$WITH_COPYNES/lib"
	fi
else
	use_copynes=no
fi
AC_MSG_RESULT([$use_copynes])


# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
if test "x$use_copynes" != "xno"; then
	AC_CHECK_LIB([copynes], [copynes_open],[AC_DEFINE(HAVE_LIBCOPYNES,1,"have libcopynes available")])
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h termio.h unistd.h getopt.h unistd.h])

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
                 copynesl/Makefile
		 trk_log/Makefile
		 settings/Makefile
		 data/Makefile
		 data/plugins/Makefile
		 man/Makefile])

AC_CONFIG_SUBDIRS([cartctl])

AC_OUTPUT


