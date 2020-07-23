/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Non-thread-safe hash */

#pragma once

#include <stdint.h>
#include <taskworks_internal.h>

typedef struct TWI_Hash_node_t {
	void *data;
	struct TWI_Hash_node_t *next;
} TWI_Hash_node_t;

typedef struct TWI_Hash_t {
	size_t idx_mask;
	TWI_Hash_node_t **buckets;
} TWI_Hash_t;
typedef TWI_Hash_t *TWI_Hash_handle_t;

terr_t TWI_Hash_create (unsigned int idx_len, TWI_Hash_handle_t *h);
terr_t TWI_Hash_init (TWI_Hash_handle_t h, unsigned int idx_len);
terr_t TWI_Hash_finalize (TWI_Hash_handle_t h);
terr_t TWI_Hash_free (TWI_Hash_handle_t h);
terr_t TWI_Hash_insert (TWI_Hash_handle_t h, void *data);
TWI_Bool_t TWI_Hash_exists (TWI_Hash_handle_t h, void *data);
terr_t TWI_Hash_del (TWI_Hash_handle_t h, void *data);
