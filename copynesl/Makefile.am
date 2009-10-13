AM_CFLAGS=-g -Wall -pedantic -W -Wno-unused -ansi -DSYSCONFDIR=\"@sysconfdir@\" -DDATAROOTDIR=\"@datarootdir@\"

bin_PROGRAMS=copynesl
copynesl_SOURCES=copynesl.c options.c plugins.c output.c unif.c
if HAVE_LIBCOPYNES
copynesl_SOURCES+=copynes.c 
endif

noinst_HEADERS=options.h plugins.h nes.h errorcodes.h output.h unif.h

LDADD=../trk_log/libtrk_log.a ../settings/libsettings.a ../cartctl/cartctl/libcartctl.la
if HAVE_LIBCOPYNES
LDADD+=-lcopynes
endif
INCLUDES=-I@top_srcdir@ -I@top_srcdir@/cartctl
MEASOURCES=auto
