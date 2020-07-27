/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* No wait linked list */

#pragma once

#include <opa_primitives.h>

#include "twi_rwlock.h"

typedef struct TWI_Nb_list_node_t {
	void *data;
	OPA_ptr_t next;
	// OPA_ptr_t pre;
	OPA_ptr_t next_free;
} TWI_Nb_list_node_t;
typedef TWI_Nb_list_node_t *TWI_Nb_list_itr_t;

typedef struct TWI_Nb_list_t {
	TWI_Rwlock_t lock;
	TWI_Nb_list_node_t head;
	TWI_Nb_list_node_t tail;
	OPA_int_t ref;
} TWI_Nb_list_t;
typedef TWI_Nb_list_t *TWI_Nb_list_handle_t;

#define TWI_Nb_list_INIT_SIZE		 32
#define TWI_Nb_list_ALLOC_MULTIPLIER 20

#define TWI_Nb_list_STAT_INVAL 0x1	// Not initialized
#define TWI_Nb_list_STAT_RDY   0x2	// No one using
#define TWI_Nb_list_STAT_ACC   0x4	// Accessing, no modify
#define TWI_Nb_list_STAT_MOD   0x8	// Modifying, no other thread allowed

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

/* Thread safe List */

#include <stdint.h>

#include "taskworks_internal.h"
#include "twi_nb_list.h"

#define DEL_FLAG  0x01ULL
#define FLAG_MASK 0xfffffffffffffff4ULL

#define MARK_DEL(A)	 ((TWI_Nb_list_itr_t) ((uint64_t) (A) | DEL_FLAG))
#define GET_PTR(A)	 ((TWI_Nb_list_itr_t) ((uint64_t) (A)&FLAG_MASK))
#define IS_SET(A, B) ((uint64_t) (A)&B)
#define ONDEL(A)	 IS_SET (OPA_load_ptr (&(A->next)), DEL_FLAG)
#define NEXT(A)		 GET_PTR (OPA_load_ptr (&(A->next)))
#define PRE(A)		 GET_PTR (OPA_load_ptr (&(A->pre)))

terr_t TWI_Nb_list_create (TWI_Nb_list_handle_t *l);
terr_t TWI_Nb_list_init (TWI_Nb_list_handle_t l);
terr_t TWI_Nb_list_finalize (TWI_Nb_list_handle_t l);
terr_t TWI_Nb_list_free (TWI_Nb_list_handle_t l);
terr_t TWI_Nb_list_insert_at (TWI_Nb_list_handle_t l,
							  TWI_Nb_list_itr_t pos,
							  void *data,
							  TWI_Bool_t *successp);
terr_t TWI_Nb_list_insert_front (TWI_Nb_list_handle_t l, void *data);
terr_t TWI_Nb_list_del_at (TWI_Nb_list_handle_t l, TWI_Nb_list_itr_t pos, int *successp);
terr_t TWI_Nb_list_del (TWI_Nb_list_handle_t l, void *data);
TWI_Nb_list_itr_t TWI_Nb_list_find (TWI_Nb_list_handle_t l, void *data);
TWI_Nb_list_itr_t TWI_Nb_list_begin (TWI_Nb_list_handle_t l);
TWI_Nb_list_itr_t TWI_Nb_list_end (TWI_Nb_list_handle_t l);
TWI_Nb_list_itr_t TWI_Nb_list_next (TWI_Nb_list_itr_t itr);
TWI_Nb_list_itr_t TWI_Nb_list_pre (TWI_Nb_list_handle_t l, TWI_Nb_list_itr_t itr);
void TWI_Nb_list_inc_ref (TWI_Nb_list_handle_t l);
void TWI_Nb_list_dec_ref (TWI_Nb_list_handle_t l);