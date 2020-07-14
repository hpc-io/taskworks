# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of Taskworks. The full Taskworks copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUFFIXES = .o .c .h .m4

AM_CPPFLAGS  =  -I${top_srcdir}/src/include \
                -I${top_srcdir}/src/common

# Our main target, the task engine library
noinst_LTLIBRARIES=libtwcommon.la

# library sources
libtwcommon_la_SOURCES =    twi_mem.c \
                            twi_time.c \
                            twi_mutex.c \
                            twi_rwlock.c \
                            twi_vector.c \
                            twi_list.c


# Allow running make tests without make
tests-local: all