## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I ../../../../m4

srcdir = @srcdir@
top_srcdir = @top_srcdir@
VPATH = @srcdir@
INCLUDES = -I. -I@srcdir@/include
DEFS = @DEFS@ @ARCH_DEFS@ -Wall
EXTRALDFLAGS = @EXTRA_LDFLAGS@

TESTS_ENVIRONMENT = $(srcdir)/../run_occ21_test.py $(top_srcdir)/occ21
TESTS = chan_signal_decl.occ \
	chan_array_signal_decl.occ \
	variable_decl.occ

