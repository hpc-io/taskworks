# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of Taskworks. The full Taskworks copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUFFIXES = .o .c .h .m4

AM_CPPFLAGS  =  -I${top_srcdir}/src \
                -I${top_srcdir}/src/include \
                -I${top_srcdir}/src/common \
                -I${top_srcdir}/src/driver \
                -I${top_srcdir}/src/tal \
                -I${top_srcdir}/src/eal
                
# Our main target, the task engine library
noinst_LTLIBRARIES=libtwdispatcher.la

# library sources
libtwdispatcher_la_SOURCES =    tw.c \
                                    engine.c \
                                    task.c \
                                    dep.c \
                                    err.c \
                                    event.c
                                    poll.c 

# M4 hook
# M4_SRCS  = 
# M4FLAGS += -I${top_srcdir}/m4
# $(M4_SRCS:.m4=.c): Makefile
# .m4.c:
#       $(M4) $(AM_M4FLAGS) $(M4FLAGS) $< >$@
# BUILT_SOURCES = $(M4_SRCS:.m4=.c)
# CLEANFILES = $(M4_SRCS:.m4=.c)

# Allow running make tests without make
tests-local: all