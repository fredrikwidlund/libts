ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
COMMON_FLAGS = -g -O3 -flto -fuse-linker-plugin -I../src -L..
AM_CFLAGS = -std=gnu11 $(COMMON_FLAGS)
AM_LDFLAGS = $(COMMON_FLAGS) -static

bin_PROGRAMS = remux_packets filter

remux_packets_SOURCES = remux_packets.c
remux_packets_LDADD = ../libts.la -ldynamic

filter_SOURCES = filter.c
filter_LDADD = ../libts.la -ldynamic

MAINTAINERCLEANFILES = Makefile.in
