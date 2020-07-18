/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Thread safe linked list */

#pragma once

#include <opa_primitives.h>

#include "twi_rwlock.h"

typedef struct TWI_List_node_t {
	void *data;
	OPA_ptr_t next;
	OPA_ptr_t pre;
} TWI_List_node_t;
typedef TWI_List_node_t *TWI_List_itr_t;

typedef struct TWI_List_t {
	TWI_Rwlock_t lock;
	OPA_ptr_t head;
	OPA_ptr_t tail;
} TWI_List_t;
typedef TWI_List_t *TWI_List_handle_t;

#define TWI_List_INIT_SIZE		  32
#define TWI_List_ALLOC_MULTIPLIER 20

#define TWI_List_STAT_INVAL 0x1	 // Not initialized
#define TWI_List_STAT_RDY	0x2	 // No one using
#define TWI_List_STAT_ACC	0x4	 // Accessing, no modify
#define TWI_List_STAT_MOD	0x8	 // Modifying, no other thread allowed

terr_t TWI_List_create (TWI_List_handle_t *l);
terr_t TWI_List_free (TWI_List_handle_t l);
terr_t TWI_List_insert_front (TWI_List_handle_t l, void *data);
// terr_t TWI_List_insert_front_at (TWI_List_itr_t itr, void *data);
terr_t TWI_List_erase (TWI_List_handle_t l, void *data);
TWI_List_itr_t TWI_List_begin (TWI_List_handle_t l);
TWI_List_itr_t TWI_List_next (TWI_List_itr_t itr);
TWI_List_itr_t TWI_List_pre (TWI_List_itr_t itr);