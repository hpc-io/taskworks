/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Common internal routines */

#pragma once

#include <stdint.h>

typedef void *TW_Handle_t;

typedef int64_t ttime_t;  // # microsecond since

ttime_t TWI_Time_now ();

void *TWI_Malloc (size_t size);
void *TWI_Realloc (void *old_ptr, size_t size);
void TWI_Free (void *ptr);
