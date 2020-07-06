# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of TaskEngine. The full TaskEngine copyright notice,    
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.                                                                     

SUBDIRS = dispatcher driver common
DIST_SUBDIRS = dispatcher driver common

# Our main target, the task engine library
lib_LTLIBRARIES=libtw.la
libtw_la_SOURCES = 
libtw_la_LIBADD  =  dispatcher/libtwdispatcher.la \
                    common/libtwcommon.la

include_HEADERS =   include/taskworks.h \
                        include/tw_dispatcher.h \
                        include/tw_engine.h \
                        include/tw_task.h \
                        include/tw_event.h \
                        include/tw_err.h
