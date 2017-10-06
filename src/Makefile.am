## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libdot.la


libdot_la_SOURCES = \
	dot.c

libdot_la_CFLAGS = -DLARGEFILE64_SOURCE=1 \
	-DFILE_OFFSET_BITS=64 

libdot_la_LDFLAGS = 

libdot_la_LIBADD = 


if PLATFORM_WIN32
libdot_la_LDFLAGS += -no-undefined
endif

if NATIVE_WIN32
libdot_la_LDFLAGS += -export-dynamic
endif

include_HEADERS = \
	dot.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdot-0.1.pc

EXTRA_DIST = \
	libdot-0.1.pc.in
