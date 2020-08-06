/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,
 *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Auxiliary functions for debugging */

#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#endif

#include "debug.h"

#ifdef _WIN32
int TWI_Get_tid (void) { return (int)GetThreadId (); }
#else
int TWI_Get_tid (void) { return (int)gettid (); }
#endif

int TWI_Debug_level = 0;