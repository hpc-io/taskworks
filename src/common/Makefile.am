# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of TaskEngine. The full TaskEngine copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUFFIXES = .o .c .h .m4

AM_CPPFLAGS  = -I${top_srcdir}/src/include

# Our main target, the task engine library
noinst_LTLIBRARIES=libtwcommon.la

# library sources
libtwcommon_la_SOURCES =    mem.c \
                            time.c 
