ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
COMMON_FLAGS = -g -O3 -flto -fuse-linker-plugin -I../src -L..
AM_CFLAGS = -std=gnu11 $(COMMON_FLAGS)
AM_LDFLAGS = $(COMMON_FLAGS) -static

bin_PROGRAMS = remux_packets remux_units remux_stream ts_debug_packets ts_debug_units ts_debug_stream

remux_packets_SOURCES = remux_packets.c
remux_packets_LDADD = ../libts.la -ldynamic

remux_units_SOURCES = remux_units.c
remux_units_LDADD = ../libts.la -ldynamic

remux_stream_SOURCES = remux_stream.c
remux_stream_LDADD = ../libts.la -ldynamic

ts_debug_packets_SOURCES = ts_debug_packets.c
ts_debug_packets_LDADD = ../libts.la -ldynamic

ts_debug_units_SOURCES = ts_debug_units.c
ts_debug_units_LDADD = ../libts.la -ldynamic

ts_debug_stream_SOURCES = ts_debug_stream.c
ts_debug_stream_LDADD = ../libts.la -ldynamic

MAINTAINERCLEANFILES = Makefile.in
