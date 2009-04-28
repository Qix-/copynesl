AM_CFLAGS=-g -Wall -pedantic -W -Wno-unused -ansi -DSYSCONFDIR=\"@sysconfdir@\"

noinst_LIBRARIES=libsettings.a
libsettings_a_SOURCES=settings.h settings.c dir.c array.c
noinst_HEADERS=dir.h array.h
check_PROGRAMS=test_settings test_dir test_array
TESTS=$(check_PROGRAMS)
test_settings_SOURCES=tests/test_settings.c
test_dir_SOURCES=tests/test_dir.c
test_array_SOURCES=tests/test_array.c

test_settings_LDADD=./libsettings.a ../trk_log/libtrk_log.a
test_dir_LDADD=./libsettings.a ../trk_log/libtrk_log.a
test_array_LDADD=./libsettings.a ../trk_log/libtrk_log.a
INCLUDES=-I@top_srcdir@
