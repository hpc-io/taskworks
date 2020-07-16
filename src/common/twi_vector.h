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
#include "twi_rwlock.h"

typedef struct TWI_Vector_t {
	TWI_Rwlock_t lock;
	int size;
	size_t nalloc;
	void **data;
} TWI_Vector_t;

typedef TWI_Vector_t *TWI_Vector_handle_t;

#define TWI_Vector_INIT_SIZE		32
#define TWI_Vector_ALLOC_MULTIPLIER 20

#define TWI_Vector_STAT_INVAL 0x1  // Not initialized
#define TWI_Vector_STAT_RDY	  0x2  // No one using
#define TWI_Vector_STAT_ACC	  0x4  // Accessing, no modify
#define TWI_Vector_STAT_MOD	  0x8  // Modifying, no other thread allowed

terr_t TWI_Vector_create (TWI_Vector_handle_t *v);
terr_t TWI_Vector_init (TWI_Vector_handle_t v);
terr_t TWI_Vector_finalize (TWI_Vector_handle_t v);
terr_t TWI_Vector_free (TWI_Vector_handle_t v);
terr_t TWI_Vector_read (TWI_Vector_handle_t v, int index, void **data);
terr_t TWI_Vector_write (TWI_Vector_handle_t v, int index, void *data);
terr_t TWI_Vector_erase_at (TWI_Vector_handle_t v, int idx);
terr_t TWI_Vector_erase (TWI_Vector_handle_t v, void *data);
int TWI_Vector_find (TWI_Vector_handle_t v, void *data);
terr_t TWI_Vector_push_back (TWI_Vector_handle_t v, void *data);
size_t TWI_Vector_size (TWI_Vector_handle_t v);
terr_t TWI_Vector_resize (TWI_Vector_handle_t v, size_t size);