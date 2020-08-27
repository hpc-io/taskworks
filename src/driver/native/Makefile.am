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
                -I${top_srcdir}/src/tal\
                -I${top_srcdir}/src/eal


# Argobots driver
noinst_LTLIBRARIES=libtwnative.la
libtwnative_la_SOURCES =    twnative.c \
                                twnative_engine.c \
                                    twnative_engine_int.c \
                                    twnative_engine_sched.c \
                                twnative_event.c \
                                    twnative_event_int.c \
                                twnative_task.c \
                                    twnative_task_int.c
                                
# Allow running make tests without make
tests-local: all