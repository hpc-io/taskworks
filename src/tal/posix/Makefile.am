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
                -I${top_srcdir}/src/tal

# Argobots driver
noinst_LTLIBRARIES=libtwposix.la
libtwposix_la_SOURCES =     err.c \
                            twposix.c \
                                twposix_mutex.c \
                                twposix_rwlock.c \
                                twposix_sem.c \
                                twposix_tls.c

# Allow running make tests without make
tests-local: all