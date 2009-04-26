AM_CFLAGS=-g -Wall -pedantic -W -Wno-unused -ansi -DSYSCONFDIR=\"@sysconfdir@\" -DDATAROOTDIR=\"@datarootdir@\"

bin_PROGRAMS=copynesl
copynesl_SOURCES=copynesl.c options.c plugins.c copynes.c dump_cart.c 
noinst_HEADERS=options.h plugins.h nes.h

LDADD=../trk_log/libtrk_log.a ../settings/libsettings.a ../cartctl/cartctl/libcartctl.la -lcopynes
INCLUDES=-I@top_srcdir@ -I@top_srcdir@/cartctl
MEASOURCES=auto
