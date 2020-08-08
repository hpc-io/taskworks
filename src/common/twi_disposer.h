/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Cross-platform read write lock wrapper */

#pragma once

#include "twi_err.h"
#include "twi_mutex.h"
#include "twi_tls.h"

typedef void (*TW_Trash_handler_t) (void *data);

typedef struct TWI_Trash_t {
	void *obj;					 // Item
	TW_Trash_handler_t handler;	 // Freeing handler
	int ts;						 // Time stamp when the item is mark for freeing
	struct TWI_Trash_t *next;
} TWI_Trash_t;

typedef struct TWI_Disposer_t {
	TWI_Mutex_handle_t lock;

	OPA_int_t ts;  // time stamp
	int tmin;	   // Min time agreed by all threads

	int nt_alloc;
	int nt;	   // Num participating threads
	int *tss;  // Time stamp of all threads

	TWI_Tls_t cnt;	// How many times the thread has joined the system, allow recursive join
	TWI_Tls_t tid;	// Thread ID in this system

	OPA_ptr_t head;
} TWI_Disposer_t;

typedef TWI_Disposer_t *TWI_Disposer_handle_t;

TWI_Disposer_handle_t TWI_Disposer_create (void);
terr_t TWI_Disposer_init (TWI_Disposer_handle_t dp);
void TWI_Disposer_finalize (TWI_Disposer_handle_t dp);
void TWI_Disposer_free (TWI_Disposer_handle_t dp);
terr_t TWI_Disposer_dispose (TWI_Disposer_handle_t dp,
							 void *obj,
							 TW_Trash_handler_t handler);  // Dispose obj with handler
void TWI_Disposer_flush (TWI_Disposer_handle_t dp);
void TWI_Disposer_join (TWI_Disposer_handle_t dp);	 // A thread joins the system
void TWI_Disposer_leave (TWI_Disposer_handle_t dp);	 // A thread leves the system
