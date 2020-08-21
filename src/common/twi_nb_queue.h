/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* No wait lqueue */

#pragma once

#include <opa_primitives.h>

#include "common.h"
#include "twi_rwlock.h"

typedef struct TWI_Nb_queue_node_t {
	void *data;
	struct TWI_Nb_queue_node_t *pre;  // An estimate, may not be real pre
	OPA_ptr_t next;
	OPA_ptr_t next_free;
} TWI_Nb_queue_node_t;
typedef TWI_Nb_queue_node_t *TWI_Nb_queue_itr_t;

typedef struct TWI_Nb_queue_t {
	TWI_Nb_queue_node_t head;
	OPA_ptr_t tail;	 // Only a ref
	OPA_int_t ref;
	OPA_ptr_t recycle;
	TWI_Nb_queue_node_t *free;
} TWI_Nb_queue_t;
typedef TWI_Nb_queue_t *TWI_Nb_queue_handle_t;

#define DEL_FLAG  0x01ULL
#define FLAG_MASK 0xfffffffffffffff4ULL

terr_t TWI_Nb_queue_create (TWI_Nb_queue_handle_t *l);
terr_t TWI_Nb_queue_init (TWI_Nb_queue_handle_t l);
void TWI_Nb_queue_finalize (TWI_Nb_queue_handle_t l);
void TWI_Nb_queue_free (TWI_Nb_queue_handle_t l);
terr_t TWI_Nb_queue_push (TWI_Nb_queue_handle_t l, void *data);
void TWI_Nb_queue_pop (TWI_Nb_queue_handle_t l, void **data, TWI_Bool_t *successp);
