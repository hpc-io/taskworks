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
                -I${top_srcdir}/src/eal

# Argobots driver
noinst_LTLIBRARIES=libtwlibevt.la
libtwlibevt_la_SOURCES =    err.c \
                            twlibevt.c \
                                twlibevt_loop.c \
                                twlibevt_event.c \
                                twlibevti_loop.c \
                                twlibevti_event.c

# Allow running make tests without make
tests-local: all