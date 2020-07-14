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

/* Thread safe Vector */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <opa_primitives.h>

#include "taskworks_internal.h"

#define TWI_Vector_INIT_SIZE		32
#define TWI_Vector_ALLOC_MULTIPLIER 20

#define TWI_Vector_STAT_INVAL 0x1  // Not initialized
#define TWI_Vector_STAT_RDY	  0x2  // No one using
#define TWI_Vector_STAT_ACC	  0x4  // Accessing, no modify
#define TWI_Vector_STAT_MOD	  0x8  // Modifying, no other thread allowed

#define TWI_Vector_WAIT_STAT(A, B)                                              \
	{                                                                           \
		int cur_stat;                                                           \
		while (1) {                                                             \
			cur_stat = OPA_load_int (&(v->status));                             \
			if (cur_stat & A) {                                                 \
				if (cur_stat == OPA_cas_int (&(v->status), cur_stat, B)) break; \
			}                                                                   \
		}                                                                       \
	}

terr_t TWI_Vector_init (TWI_Vector_t *v) {
	terr_t err = TW_SUCCESS;

	TWI_Rwlock_create (&(v->lock));
	v->size	  = 0;
	v->nalloc = TWI_Vector_INIT_SIZE;
	v->data	  = TWI_Malloc (v->nalloc * sizeof (TW_Handle_t));
	CHK_PTR (v->data)
err_out:;
	return err;
}

terr_t TWI_Vector_free (TWI_Vector_t *v) {
	TWI_Rwlock_wlock (&(v->lock));

	TWI_Free (v->data);
	v->data = NULL;

	TWI_Rwlock_wunlock (&(v->lock));
	TWI_Rwlock_free (&(v->lock));

	return TW_SUCCESS;
}

terr_t TWI_Vector_read (TWI_Vector_t *v, int64_t index, void **data) {
	TWI_Rwlock_rlock (&(v->lock));

	if (index < 0 || index > v->size) { RET_ERR (TW_ERR_INVAL) }

	*data = v->data[index];

	TWI_Rwlock_runlock (&(v->lock));

	return TW_SUCCESS;
}
terr_t TWI_Vector_write (TWI_Vector_t *v, int64_t index, void *data) {
	TWI_Rwlock_rlock (&(v->lock));

	if (index < 0 || index > v->size) { RET_ERR (TW_ERR_INVAL) }

	v->data[index] = data;

	TWI_Rwlock_runlock (&(v->lock));

	return TW_SUCCESS;
}

terr_t TWI_Vector_erase_at (TWI_Vector_t *v, int64_t idx) {
	terr_t err = TW_SUCCESS;
	int i;

	TWI_Rwlock_wlock (&(v->lock));

	if (idx < 0 || idx > v->size) { RET_ERR (TW_ERR_INVAL) }

	/* Pull in the next one */
	for (i = idx; i < v->size - 1; i++) { v->data[i] = v->data[i + 1]; }

	/* Reduce size */
	v->size--;

	TWI_Rwlock_wunlock (&(v->lock));

err_out:;
	return err;
}

terr_t TWI_Vector_erase (TWI_Vector_t *v, void *data) {
	terr_t err = TW_SUCCESS;
	int i;

	TWI_Rwlock_wlock (&(v->lock));

	/* Pull in the next one */
	for (i = 0; i < v->size; i++) {
		if (v->data[i] == data) {
			for (; i < v->size - 1; i++) { v->data[i] = v->data[i + 1]; }
			break;
		}
	}

	/* Reduce size */
	v->size--;

	TWI_Rwlock_wunlock (&(v->lock));

err_out:;
	return err;
}

int64_t TWI_Vector_find (TWI_Vector_t *v, void *data) {
	int64_t i;

	TWI_Rwlock_rlock (&(v->lock));

	/* Search through */
	for (i = 0; i < v->size; i++) {
		if (v->data[i] == data) { return i; }
	}

	TWI_Rwlock_runlock (&(v->lock));

	return -1;
}

terr_t TWI_Vector_push_back (TWI_Vector_t *v, void *data) {
	terr_t err = TW_SUCCESS;

	TWI_Rwlock_wlock (&(v->lock));

	/* Increase size if needed */
	if (v->nalloc == v->size) {
		v->nalloc *= TWI_Vector_ALLOC_MULTIPLIER;
		v->data = TWI_Realloc (v->data, v->nalloc * sizeof (void *));
		CHK_PTR (v->data)
	}

	/* Write at back */
	v->data[v->size++] = data;

	TWI_Rwlock_wunlock (&(v->lock));

err_out:;
	return err;
}

size_t TWI_Vector_size (TWI_Vector_t *v) { return v->size; }

size_t TWI_Vector_resize (TWI_Vector_t *v, size_t size) {
	terr_t err = TW_SUCCESS;

	TWI_Rwlock_wlock (&(v->lock));

	if (v->nalloc == v->size) {
		v->nalloc *= TWI_Vector_ALLOC_MULTIPLIER;
		v->data = TWI_Realloc (v->data, v->nalloc * sizeof (void *));
		CHK_PTR (v->data)
	}

	TWI_Rwlock_wunlock (&(v->lock));

err_out:;
	return err;
}
