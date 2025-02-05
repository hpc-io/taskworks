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
#ifdef TWI_DEBUG
#include <execinfo.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <opa_primitives.h>

#include "twi_ts_vector.h"

#ifdef TWI_DEBUG
#define CHECK_IDX                                                         \
	{                                                                     \
		if (index < 0 || index >= (int)TWI_Ts_vectori_get_size (v)) {     \
			printf ("Vector index out of bound: size = %d, index = %d\n", \
					(int)TWI_Ts_vectori_get_size (v), index);             \
			abort ();                                                     \
		}                                                                 \
	}
#else
#define CHECK_IDX \
	{}
#endif

#define TWI_VECTOR_INIT_SIZE		32
#define TWI_VECTOR_ALLOC_MULTIPLIER 20
#ifdef TWI_DEBUG
#define SET_OWNER                   \
	{                               \
		v->owner = TWI_Get_tid ();  \
		backtrace (v->trace, 1024); \
	}
#define CLR_OWNER \
	{ v->owner = -1; }
#define CHK_OWNER                                     \
	{                                                 \
		if (v->owner == TWI_Get_tid ()) { abort (); } \
	}
#else
#define SET_OWNER \
	{}
#define CHK_OWNER \
	{}
#define CLR_OWNER \
	{}
#endif
#define RLOCK                          \
	{                                  \
		CHK_OWNER                      \
		TWI_Rwlock_rlock (&(v->lock)); \
		locked = 1;                    \
		SET_OWNER                      \
	}

#define WLOCK                          \
	{                                  \
		CHK_OWNER                      \
		TWI_Rwlock_wlock (&(v->lock)); \
		locked = 2;                    \
		SET_OWNER                      \
	}

#define RUNLOCK                              \
	{                                        \
		if (locked == 1) {                   \
			TWI_Rwlock_runlock (&(v->lock)); \
			locked = 0;                      \
			CLR_OWNER                        \
		}                                    \
	}

#define WUNLOCK                              \
	{                                        \
		if (locked == 2) {                   \
			TWI_Rwlock_wunlock (&(v->lock)); \
			locked = 0;                      \
			CLR_OWNER                        \
		}                                    \
	}

#define UNLOCK                                   \
	{                                            \
		switch (locked) {                        \
			case 1:                              \
				TWI_Rwlock_runlock (&(v->lock)); \
				locked = 0;                      \
				break;                           \
			case 2:                              \
				TWI_Rwlock_wunlock (&(v->lock)); \
				locked = 0;                      \
				break;                           \
			default:;                            \
				abort ();                        \
		}                                        \
		CLR_OWNER                                \
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
	while (new_size < (size_t)size) { new_size *= TWI_VECTOR_ALLOC_MULTIPLIER; }

	ptr = TWI_Realloc (v->data, new_size * sizeof (void *));
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

TWI_Ts_vector_handle_t TWI_Ts_vector_create (void) {
	terr_t err				  = TW_SUCCESS;
	TWI_Ts_vector_handle_t vp = NULL;

	vp = (TWI_Ts_vector_handle_t)TWI_Malloc (sizeof (TWI_Ts_vector_t));
	CHECK_PTR (vp)

	err = TWI_Ts_vector_init (vp);
	CHECK_ERR

err_out:;
	if (err) {
		if (vp) {
			TWI_Free (vp);
			vp = NULL;
		}
	}
	return vp;
}

void TWI_Ts_vector_free (TWI_Ts_vector_handle_t v) {
	TWI_Ts_vector_finalize (v);
	TWI_Free (v);
}

terr_t TWI_Ts_vector_init (TWI_Ts_vector_handle_t v) {
	terr_t err = TW_SUCCESS;

	err = TWI_Rwlock_init (&(v->lock));
	CHECK_ERR
	OPA_store_int (&(v->size), 0);
	v->nalloc = TWI_VECTOR_INIT_SIZE;
	v->data	  = TWI_Malloc (v->nalloc * sizeof (TW_Handle_t));
	CHECK_PTR (v->data)
#ifdef TWI_DEBUG
	v->owner = -1;
#endif

err_out:;
	if (err) {
		TWI_Free (v->data);
		TWI_Rwlock_finalize (&(v->lock));
	}
	return err;
}

void TWI_Ts_vector_finalize (TWI_Ts_vector_handle_t v) {
	TWI_Free (v->data);
	v->data = NULL;
	TWI_Rwlock_finalize (&(v->lock));
}

void *TWI_Ts_vector_read (TWI_Ts_vector_handle_t v, int index) {
	void *data;
	int locked = 0;

	CHECK_IDX
	RLOCK;

	data = v->data[index];

	RUNLOCK;

	return data;
}
void TWI_Ts_vector_write (TWI_Ts_vector_handle_t v, int index, void *data) {
	int locked = 0;

	CHECK_IDX

	RLOCK;

	v->data[index] = data;

	RUNLOCK;
}

void TWI_Ts_vector_erase_at (TWI_Ts_vector_handle_t v, int index) {
	int i;
	int size;
	int locked = 0;

	CHECK_IDX

	WLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	/* Pull in the next one */
	for (i = index; i < size - 1; i++) { v->data[i] = v->data[i + 1]; }

	/* Reduce size */
	OPA_decr_int (&(v->size));

	WUNLOCK;
}

int TWI_Ts_vector_erase (TWI_Ts_vector_handle_t v, void *data) {
	int i, j;
	int size;
	int locked = 0;

	WLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	/* Search for item */
	for (i = size - 1; i >= 0; i--) {
		if (v->data[i] == data) {
			/* Move items after */
			for (j = i; j < size - 1; j++) { v->data[j] = v->data[j + 1]; }
			/* Reduce size */
			OPA_decr_int (&(v->size));
			break;
		}
	}

	WUNLOCK;

	return i;
}

int TWI_Ts_vector_swap_erase (TWI_Ts_vector_handle_t v, void *data) {
	int i;
	int size;
	int locked = 0;

	WLOCK;

	size = (int)TWI_Ts_vectori_get_size (v);

	/* Search for item */
	for (i = size - 1; i >= 0; i--) {
		if (v->data[i] == data) {
			/* Swap with last */
			v->data[i] = v->data[size - 1];

			/* Reduce size */
			OPA_decr_int (&(v->size));

			break;
		}
	}

	WUNLOCK;

	return i;
}

int TWI_Ts_vector_find (TWI_Ts_vector_handle_t v, void *data) {
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

void TWI_Ts_vector_lock (TWI_Ts_vector_handle_t v) {
	CHK_OWNER;
	TWI_Rwlock_rlock (&(v->lock));
	SET_OWNER;
}

void TWI_Ts_vector_unlock (TWI_Ts_vector_handle_t v) {
	TWI_Rwlock_runlock (&(v->lock));
	CLR_OWNER;
}

TWI_Ts_vector_itr_t TWI_Ts_vector_begin (TWI_Ts_vector_handle_t v) { return v->data; }

TWI_Ts_vector_itr_t TWI_Ts_vector_end (TWI_Ts_vector_handle_t v) {
	return v->data + OPA_load_int (&(v->size));
}