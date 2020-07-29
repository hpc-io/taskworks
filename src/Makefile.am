# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of Taskworks. The full Taskworks copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUBDIRS = dispatcher driver common
DIST_SUBDIRS = ${SUBDIRS}

# Our main target, the task engine library
lib_LTLIBRARIES=libtw.la
libtw_la_SOURCES = 
libtw_la_LIBADD  =  dispatcher/libtwdispatcher.la \
                    common/libtwcommon.la

if ENABLE_ABT
   libtw_la_LIBADD += driver/argobots/libtwabt.la
endif
if ENABLE_LIBEVENT
   libtw_la_LIBADD += driver/libevent/libtwlibevt.la
endif



include_HEADERS =   include/taskworks.h \
                        include/tw_dispatcher.h \
                        include/tw_engine.h \
                        include/tw_task.h \
                        include/tw_event.h \
                        include/tw_err.h

# Allow running make tests without make
tests-local: all