# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of Taskworks. The full Taskworks copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUFFIXES = .o .c

AM_DEFAULT_SOURCE_EXT = .c

AM_CPPFLAGS  = -I$(top_srcdir)/src/include
AM_CPPFLAGS += -I$(top_srcdir)/test/common

LDADD  =    $(top_builddir)/src/libtw.la \
            $(top_builddir)/test/common/libtwtestcommon.la

TESTPROGRAMS =  init \
                engine \
                file \
                timer \
                socket \
                poll

if HAVE_MPI
   TESTPROGRAMS += mpi
endif

TEST_DRIVERS= "NATIVE"
TEST_EVTDRIVERS=

if HAVE_ABT
   TEST_DRIVERS += "ABT"
endif

if HAVE_LIBEVENT
   TEST_EVTDRIVERS += "LIBEVENT"
endif

AM_TESTS_ENVIRONMENT = \
DRIVERS="${TEST_DRIVERS}" \
EVTDRIVERS="${TEST_EVTDRIVERS}"

# mutual_dep

# When we need m4
# M4FLAGS += -I${top_srcdir}/m4
# $(M4_SRCS:.m4=.c): Makefile
# .m4.c:
#   $(M4) $(AM_M4FLAGS) $(M4FLAGS) $< >$@
# CLEANFILES += $(M4_SRCS:.m4=.c)
# 

check_PROGRAMS = $(TESTPROGRAMS)

TESTS = $(TESTPROGRAMS)
TEST_EXTENSIONS = .sh
LOG_COMPILER = $(srcdir)/log_comp.sh
SH_LOG_COMPILER =

# build check targets but not invoke
tests: all $(check_PROGRAMS)

.PHONY:
