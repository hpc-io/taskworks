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

#include "twi_ts_vector.h"

#define TWI_VECTOR_INIT_SIZE		32
#define TWI_VECTOR_ALLOC_MULTIPLIER 20

#define RLOCK                                \
	{                                        \
		err = TWI_Rwlock_rlock (&(v->lock)); \
		CHECK_ERR                            \
		locked = 1;                          \
	}

#define WLOCK                                \
	{                                        \
		err = TWI_Rwlock_rlock (&(v->lock)); \
		CHECK_ERR                            \
		locked = 2;                          \
	}

#define RUNLOCK                                    \
	{                                              \
		if (locked == 1) {                         \
			err = TWI_Rwlock_runlock (&(v->lock)); \
			if (err == TW_SUCCESS) locked = 0;     \
		}                                          \
	}

#define WUNLOCK                                    \
	{                                              \
		if (locked == 2) {                         \
			err = TWI_Rwlock_wunlock (&(v->lock)); \
			if (err == TW_SUCCESS) locked = 0;     \
		}                                          \
	}

#define UNLOCK                                         \
	{                                                  \
		switch (locked) {                              \
			case 1:                                    \
				err = TWI_Rwlock_runlock (&(v->lock)); \
				if (err == TW_SUCCESS) locked = 0;     \
				break;                                 \
			case 2:                                    \
				err = TWI_Rwlock_wunlock (&(v->lock)); \
				if (err == TW_SUCCESS) locked = 0;     \
				break;                                 \
			default:;                                  \
		}                                              \
	}

/*
#define TWI_Ts_vector_STAT_INVAL 0x1  // Not initialized
#define TWI_Ts_vector_STAT_RDY	  0x2  // No one using
#define TWI_Ts_vector_STAT_ACC	  0x4  // Accessing, no modify
#define TWI_Ts_vector_STAT_MOD	  0x8  // Modifying, no other thread allowed

#define TWI_Ts_vector_WAIT_STAT(A, B)                                              \
	{                                                                           \
		int cur_stat;                                                           \
		while (1) {                                                             \
			cur_stat = OPA_load_int (&(v->status));                             \
			if (cur_stat & A) {                                                 \
				if (cur_stat == OPA_cas_int (&(v->status), cur_stat, B)) break; \
			}                                                                   \
		}                                                                       \
	}
*/

inline static terr_t TWI_Ts_vectori_expand (TWI_Ts_vector_handle_t v, size_t size) {
	terr_t err = TW_SUCCESS;
	int locked = 0;
	size_t new_size;
	void *ptr;

	WLOCK;

	new_size = v->nalloc;
	while (new_size < (size_t)size) { v->nalloc *= TWI_VECTOR_ALLOC_MULTIPLIER; }

	ptr = TWI_Realloc (v->data, v->nalloc * sizeof (void *));
	CHECK_PTR (ptr)

	v->data	  = ptr;
	v->nalloc = new_size;

err_out:;
	UNLOCK;

	return err;
}

inline static size_t TWI_Ts_vectori_get_size (TWI_Ts_vector_handle_t v) {
	size_t size;
	size = (size_t)OPA_load_int (&(v->size));
	if (size > v->nalloc) size = v->nalloc;
	return size;
}

terr_t TWI_Ts_vector_create (TWI_Ts_vector_handle_t *v) {
	terr_t err = TW_SUCCESS;
	TWI_Ts_vector_handle_t vp;

	vp = (TWI_Ts_vector_handle_t)TWI_Malloc (sizeof (TWI_Ts_vector_t));
	CHECK_PTR (vp)

	err = TWI_Ts_vector_init (vp);
	CHECK_ERR

	*v = vp;

err_out:;
	if (err) {
		if (vp) { TWI_Free (vp); }
	}
	return err;
}

terr_t TWI_Ts_vector_free (TWI_Ts_vector_handle_t v) {
	terr_t err;

	err = TWI_Ts_vector_finalize (v);

	TWI_Free (v);

	return err;
}

terr_t TWI_Ts_vector_init (TWI_Ts_vector_handle_t v) {
	terr_t err = TW_SUCCESS;

	err = TWI_Rwlock_init (&(v->lock));
	CHECK_ERR
	OPA_store_int (&(v->size), 0);
	v->nalloc = TWI_VECTOR_INIT_SIZE;
	v->data	  = TWI_Malloc (v->nalloc * sizeof (TW_Handle_t));
	CHECK_PTR (v->data)

err_out:;
	if (err) { TWI_Free (v->data); }
	err = TWI_Rwlock_finalize (&(v->lock));
	return err;
}

terr_t TWI_Ts_vector_finalize (TWI_Ts_vector_handle_t v) {
	terr_t err;

	TWI_Free (v->data);
	v->data = NULL;

	err = TWI_Rwlock_finalize (&(v->lock));
	CHECK_ERR

err_out:;
	return err;
}

terr_t TWI_Ts_vector_read (TWI_Ts_vector_handle_t v, int index, void **data) {
	terr_t err;
	int locked = 0;

	RLOCK;

	if (index < 0 || index >= (int)TWI_Ts_vectori_get_size (v)) { ASSIGN_ERR (TW_ERR_INVAL) }

	*data = v->data[index];

err_out:
	RUNLOCK;

	return err;
}
terr_t TWI_Ts_vector_write (TWI_Ts_vector_handle_t v, int index, void *data) {
	terr_t err;
	int locked = 0;

	RLOCK;

	if (index < 0 || (size_t)index >= TWI_Ts_vectori_get_size (v)) { ASSIGN_ERR (TW_ERR_INVAL) }

	v->data[index] = data;

err_out:
	RUNLOCK;

	return err;
}

terr_t TWI_Ts_vector_erase_at (TWI_Ts_vector_handle_t v, int idx) {
	terr_t err = TW_SUCCESS;
	int i;
	int size;
	int locked = 0;

	WLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	if (idx < 0 || idx >= size) { ASSIGN_ERR (TW_ERR_INVAL) }

	/* Pull in the next one */
	for (i = idx; i < size - 1; i++) { v->data[i] = v->data[i + 1]; }

	/* Reduce size */
	OPA_decr_int (&(v->size));

err_out:;
	WUNLOCK;

	return err;
}

terr_t TWI_Ts_vector_erase (TWI_Ts_vector_handle_t v, void *data) {
	terr_t err = TW_SUCCESS;
	int i;
	int size;
	int locked = 0;

	WLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	/* Search for item */
	for (i = 0; i < size; i++) {
		if (v->data[i] == data) {
			for (; i < size - 1; i++) { v->data[i] = v->data[i + 1]; }
			break;
		}
	}

	/* Reduce size */
	OPA_decr_int (&(v->size));

err_out:;
	WUNLOCK;

	return err;
}

int TWI_Ts_vector_find (TWI_Ts_vector_handle_t v, void *data) {
	terr_t err = TW_SUCCESS;
	int i;
	int size;
	int locked = 0;

	RLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	/* Search through */
	for (i = 0; i < size; i++) {
		if (v->data[i] == data) { break; }
	}

	if (i > size) i = -1;

err_out:
	RUNLOCK;

	return -1;
}

terr_t TWI_Ts_vector_push_back (TWI_Ts_vector_handle_t v, void *data) {
	terr_t err = TW_SUCCESS;
	int idx;
	int locked = 0;

	RLOCK;

	// Get the ID we should write to
	idx = OPA_fetch_and_incr_int (&(v->size));

	/* Increase nalloc if needed */
	if ((size_t)idx >= v->nalloc) {
		// Release read lock so write lock can be obtained
		RUNLOCK;
		if (locked) goto err_out;

		if (err == TW_SUCCESS) { err = TWI_Ts_vectori_expand (v, (size_t)idx + 1); }

		RLOCK;
	}

	/* Write at the id we got */
	v->data[idx] = data;

err_out:
	RUNLOCK;

	return err;
}

size_t TWI_Ts_vector_size (TWI_Ts_vector_handle_t v) { return TWI_Ts_vectori_get_size (v); }

terr_t TWI_Ts_vector_resize (TWI_Ts_vector_handle_t v, size_t size) {
	terr_t err = TW_SUCCESS;

	if (v->nalloc < size) {
		err = TWI_Ts_vectori_expand (v, size);
		CHECK_ERR
	}

	OPA_store_int (&(v->size), (int)size);

err_out:;
	return err;
}
