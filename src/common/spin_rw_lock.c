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

/* Thread safe Vector */

#include <assert.h>
#include <opa_primitives.h>
#include <stdio.h>

#include "taskworks_internal.h"

#define TWI_VECTOR_STAT_RDY 0  // No one using
#define TWI_VECTOR_STAT_ACC 1  // Accessing, no modify
#define TWI_VECTOR_STAT_MOD 2  // Modifying, no other thread allowed

typedef struct TWI_Srw_lock {
	OPA_int_t status;
	size_t esize;
	int lesize;
	size_t size;
	size_t nalloc;
	void *data;
} TWI_Srw_lock;
