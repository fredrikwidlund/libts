ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
AM_CFLAGS = -std=gnu11 -g -O3 -flto -fuse-linker-plugin
AM_LDFLAGS = -static

EXTRA_DIST = \
CHANGES \
LICENSE \
README.rst

SOURCE_FILES = \
src/ts/ts_pcr.c \
src/ts/ts_ebp.c \
src/ts/ts_adaptation_field.c \
src/ts/ts_packet.c \
src/ts/ts_packets.c \
src/ts/ts_unit.c \
src/ts/ts_units.c \
src/ts/ts_psi.c \
src/ts/ts_pat.c \
src/ts/ts_pmt.c \
src/ts/ts_pes.c \
src/ts/ts_stream.c

HEADER_FILES = \
src/ts/ts_pcr.h \
src/ts/ts_ebp.h \
src/ts/ts_adaptation_field.h \
src/ts/ts_packet.h \
src/ts/ts_packets.h \
src/ts/ts_unit.h \
src/ts/ts_units.h \
src/ts/ts_psi.h \
src/ts/ts_pat.h \
src/ts/ts_pmt.h \
src/ts/ts_pes.h \
src/ts/ts_stream.h

AUTOMAKE_OPTIONS = subdir-objects
lib_LTLIBRARIES= libts.la
libts_la_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)

headerfilesdir = $(includedir)/ts
headerfiles_HEADERS = $(HEADER_FILES)

mainheaderdir = $(includedir)
mainheader_HEADERS = src/ts.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libts.pc

MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in docs/Makefile.in benchmark/Makefile.in examples/Makefile.in libts-?.?.?.tar.gz
maintainer-clean-local:; rm -rf autotools m4 libts-?.?.?

### unit tests ###

CHECK_CFLAGS = -std=gnu11 -O0 -g -ftest-coverage -fprofile-arcs
CHECK_LDADD = -L. -ldynamic_test -lcmocka
CHECK_LDFLAGS_EXTRA = -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=aligned_alloc -Wl,--wrap=abort

check_LIBRARIES = libdynamic_test.a
libdynamic_test_a_CFLAGS = $(CHECK_CFLAGS)
libdynamic_test_a_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)

check_PROGRAMS = test/ts
test_ts_CFLAGS = $(CHECK_CFLAGS)
test_ts_LDADD = $(CHECK_LDADD)
test_ts_LDFLAGS = $(CHECK_LDFLAGS_EXTRA)
test_ts_SOURCES = test/ts.c test/mock.c
dist_noinst_SCRIPTS = test/valgrind.sh test/coverage.sh

TESTS = $(check_PROGRAMS) test/coverage.sh test/valgrind.sh

CLEANFILES = {.,src/ts,test}/*.{gcno,gcda,gcov}
