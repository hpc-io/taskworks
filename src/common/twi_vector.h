/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Thread safe queue */

#pragma once

#include "common.h"
#include "taskworks_internal.h"

typedef struct TWI_Vector_t {
	void **ptr;
	void **end;
	void **base;
} TWI_Vector_t;

typedef TWI_Vector_t *TWI_Vector_handle_t;

#define TWI_STACK_INIT_SIZE	   0x400  // 1024 elements
#define TWI_STACK_SHIFT_AMOUNT 4	  // x 16 each time

#define TWI_Vector_begin(v) (v->base)
#define TWI_Vector_end(v)	(v->ptr)

TWI_Vector_handle_t TWI_Vector_create (void);
terr_t TWI_Vector_init (TWI_Vector_handle_t s);
void TWI_Vector_finalize (TWI_Vector_handle_t s);
void TWI_Vector_free (TWI_Vector_handle_t s);
size_t TWI_Vector_size (TWI_Vector_handle_t s);
terr_t TWI_Vector_resize (TWI_Vector_handle_t s, size_t size);
terr_t TWI_Vector_push_back (TWI_Vector_handle_t s, void *data);
terr_t TWI_Vector_pop_back (TWI_Vector_handle_t s, void **data);