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
#include "twi_rwlock.h"

typedef struct TWI_Ts_queue_t {
	TWI_Rwlock_t lock;
	OPA_int_t start;
	OPA_int_t end;
	int mask;
	size_t nalloc;
	void **data;
} TWI_Ts_queue_t;

typedef TWI_Ts_queue_t *TWI_Ts_queue_handle_t;

#define TWI_Ts_queue_INIT_MASK 0xFFF  // 4096 elements

TWI_Ts_queue_handle_t TWI_Ts_queue_create (void);
terr_t TWI_Ts_queue_init (TWI_Ts_queue_handle_t v);
void TWI_Ts_queue_finalize (TWI_Ts_queue_handle_t v);
void TWI_Ts_queue_free (TWI_Ts_queue_handle_t v);
terr_t TWI_Ts_queue_push (TWI_Ts_queue_handle_t v, void *data);
void TWI_Ts_queue_pop (TWI_Ts_queue_handle_t v, void **data, TWI_Bool_t *successp);