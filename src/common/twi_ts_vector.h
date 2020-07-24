/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Thread safe vector */

#pragma once

#include "common.h"
#include "taskworks_internal.h"
#include "twi_rwlock.h"

typedef struct TWI_Ts_vector_t {
	TWI_Rwlock_t lock;
	OPA_int_t size;
	size_t nalloc;
	void **data;
} TWI_Ts_vector_t;

typedef TWI_Ts_vector_t *TWI_Ts_vector_handle_t;

#define TWI_Ts_vector_INIT_SIZE		   32
#define TWI_Ts_vector_ALLOC_MULTIPLIER 20

#define TWI_Ts_vector_STAT_INVAL 0x1  // Not initialized
#define TWI_Ts_vector_STAT_RDY	 0x2  // No one using
#define TWI_Ts_vector_STAT_ACC	 0x4  // Accessing, no modify
#define TWI_Ts_vector_STAT_MOD	 0x8  // Modifying, no other thread allowed

TWI_Ts_vector_handle_t TWI_Ts_vector_create (void);
terr_t TWI_Ts_vector_init (TWI_Ts_vector_handle_t v);
void TWI_Ts_vector_finalize (TWI_Ts_vector_handle_t v);
void TWI_Ts_vector_free (TWI_Ts_vector_handle_t v);
void *TWI_Ts_vector_read (TWI_Ts_vector_handle_t v, int index);
void TWI_Ts_vector_write (TWI_Ts_vector_handle_t v, int index, void *data);
void TWI_Ts_vector_erase_at (TWI_Ts_vector_handle_t v, int idx);
int TWI_Ts_vector_swap_erase (TWI_Ts_vector_handle_t v, void *data);
int TWI_Ts_vector_erase (TWI_Ts_vector_handle_t v, void *data);
int TWI_Ts_vector_find (TWI_Ts_vector_handle_t v, void *data);
terr_t TWI_Ts_vector_push_back (TWI_Ts_vector_handle_t v, void *data);
size_t TWI_Ts_vector_size (TWI_Ts_vector_handle_t v);
terr_t TWI_Ts_vector_resize (TWI_Ts_vector_handle_t v, size_t size);
void TWI_Ts_vector_lock (TWI_Ts_vector_handle_t v);
void TWI_Ts_vector_unlock (TWI_Ts_vector_handle_t v);