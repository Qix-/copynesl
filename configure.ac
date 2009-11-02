#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.5])
AC_INIT([copynesl], [0.5.8], [cradelit@gmail.com])
AM_INIT_AUTOMAKE([copynesl], [0.5.8])
LT_INIT()
AC_CONFIG_SRCDIR([copynesl])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_WITH(inspect,
	AS_HELP_STRING([--with-inspect],[Inspect test suite errors]),
	[AC_DEFINE(INSPECT_ERRORS, 1, "inspect errors")],
	[AC_DEFINE(INSPECT_ERRORS, 0, "inspect errors")])


AC_ARG_WITH(copynes,
    AC_HELP_STRING([--with-copynes@<:@=DIR@:>@],[Include copynes hardware support (default yes)]),
    [WITH_COPYNES=$withval],[WITH_COPYNES="yes"])
	
if test "x$WITH_COPYNES" != "xno"; then
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

# Checks for libraries.
if test "x$use_copynes" != "xno"; then
	AC_SEARCH_LIBS([copynes_open], [copynes],[use_copynes=yes], [use_copynes=no])
fi
if test "x$use_copynes" != "xno"; then
	AC_DEFINE(HAVE_LIBCOPYNES,1,["have libcopynes available"])
fi
AM_CONDITIONAL([HAVE_LIBCOPYNES], [test "x$use_copynes" != "xno"])
# Check for lincopynes
AC_MSG_CHECKING(for libcopynes)
AC_MSG_RESULT([$use_copynes])


# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

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

if test "x$use_copynes" == "xno"
then
	AC_MSG_WARN([ libcopynes not found! 
	All functions that communicate with CopyNES will be disabled.  
	In order to use these functions, you will need to install libcopynes.
	use --with-copynes to force a specific location for libcopynes.])
fi

