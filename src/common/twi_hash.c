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

/* Non-thread-safe hash */

#include "twi_hash.h"

#include <stdint.h>

terr_t TWI_Hash_create (unsigned int idx_len, TWI_Hash_handle_t *h) {
	terr_t err = TW_SUCCESS;

	*h = (TWI_Hash_handle_t)TWI_Malloc (sizeof (TWI_Hash_t));
	CHECK_PTR (*h);

	err = TWI_Hash_init (*h, idx_len);
	CHECK_ERR

err_out:;
	return err;
}

terr_t TWI_Hash_free (TWI_Hash_handle_t h) {
	terr_t err = TW_SUCCESS;

	err = TWI_Hash_finalize (h);
	CHECK_ERR

	TWI_Free (h);

err_out:;
	return err;
}

terr_t TWI_Hash_init (TWI_Hash_handle_t h, unsigned int idx_len) {
	terr_t err = TW_SUCCESS;
	int i;

	h->idx_mask = 1;
	while (idx_len--) { h->idx_mask <<= 1; }

	h->buckets = TWI_Malloc (sizeof (TWI_Hash_node_t) * h->idx_mask);
	CHECK_PTR (h->buckets)

	for (i = 0; (size_t)i < h->idx_mask; i++) { h->buckets[i] = NULL; }

	h->idx_mask--;

err_out:;
	return err;
}

terr_t TWI_Hash_finalize (TWI_Hash_handle_t h) {
	terr_t err = TW_SUCCESS;
	int i;
	TWI_Hash_node_t *j, *k;

	if (!h) { ASSIGN_ERR (TW_ERR_INVAL) }

	for (i = 0; (size_t)i <= h->idx_mask; i++) {
		for (j = h->buckets[i]; j; j = k) {
			k = j->next;
			TWI_Free (j);
		}
	}

	TWI_Free (h->buckets);

err_out:;
	return err;
}

terr_t TWI_Hash_insert (TWI_Hash_handle_t h, void *data) {
	terr_t err = TW_SUCCESS;
	size_t idx;
	TWI_Hash_node_t *new_node = NULL;

	if (!h) { ASSIGN_ERR (TW_ERR_INVAL) }

	if (TWI_Hash_exists (h, data) == TWI_TRUE) ASSIGN_ERR (TW_ERR_INVAL)

	idx = (size_t)data & h->idx_mask;

	new_node = TWI_Malloc (sizeof (TWI_Hash_node_t));
	CHECK_PTR (new_node)

	new_node->data	= data;
	new_node->next	= h->buckets[idx];
	h->buckets[idx] = new_node;

err_out:;
	return err;
}

terr_t TWI_Hash_try_insert (TWI_Hash_handle_t h, void *data, TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	size_t idx;
	TWI_Hash_node_t *new_node = NULL;

	if (!h) { ASSIGN_ERR (TW_ERR_INVAL) }

	if (TWI_Hash_exists (h, data) == TWI_TRUE) {
		*success = TWI_FALSE;
		goto err_out;
	}

	idx = (size_t)data & h->idx_mask;

	new_node = TWI_Malloc (sizeof (TWI_Hash_node_t));
	CHECK_PTR (new_node)

	new_node->data	= data;
	new_node->next	= h->buckets[idx];
	h->buckets[idx] = new_node;

	*success = TWI_TRUE;

err_out:;
	return err;
}

TWI_Bool_t TWI_Hash_exists (TWI_Hash_handle_t h, void *data) {
	size_t idx;
	TWI_Hash_node_t *i;

	if (!h) { return TWI_FALSE; }

	idx = (size_t)data & h->idx_mask;

	for (i = h->buckets[idx]; i; i = i->next) {
		if (i->data == data) { break; }
	}

	if (i)
		return TWI_TRUE;
	else
		return TWI_FALSE;
}

terr_t TWI_Hash_del (TWI_Hash_handle_t h, void *data) {
	terr_t err = TW_SUCCESS;
	size_t idx;
	TWI_Hash_node_t *i, *j = NULL;

	if (!h) { ASSIGN_ERR (TW_ERR_INVAL) }

	idx = (size_t)data & h->idx_mask;
	for (i = h->buckets[idx]; i; i = i->next) {
		if (i->data == data) break;
		j = i;
	}

	if (i) {
		if (j)
			j->next = i->next;
		else {
			h->buckets[idx] = i->next;
		}

		TWI_Free (i);
	} else
		ASSIGN_ERR (TW_ERR_INVAL)

err_out:;
	return err;
}
