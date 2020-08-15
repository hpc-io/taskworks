/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Driver interface */

#pragma once

#include "taskworks_internal.h"
#include "twi_time.h"

typedef struct TW_Thread_driver_t *TW_Thread_driver_handle_t;

typedef void *(*TWTD_driver_cb_t) (void *data);

typedef struct TW_Thread_mutex_cb_t {
	terr_t (*create) (TW_handle_t *m);
	void (*free) (TW_handle_t m);
	void (*lock) (TW_handle_t m);
	void (*trylock) (TW_handle_t m, TWI_Bool_t *success);
	void (*unlock) (TW_handle_t m);
} TW_Thread_mutex_cb_t;

typedef struct TW_Thread_rwlock_cb_t {
	terr_t (*create) (TW_handle_t *m);
	void (*free) (TW_handle_t m);
	void (*rlock) (TW_handle_t m);
	void (*tryrlock) (TW_handle_t m, TWI_Bool_t *success);
	void (*runlock) (TW_handle_t m);
	void (*wlock) (TW_handle_t m);
	void (*trywlock) (TW_handle_t m, TWI_Bool_t *success);
	void (*wunlock) (TW_handle_t m);
} TW_Thread_rwlock_cb_t;

typedef struct TW_Thread_sem_cb_t {
	terr_t (*create) (TW_handle_t *s);
	void (*free) (TW_handle_t s);
	void (*dec) (TW_handle_t s);
	void (*trydec) (TW_handle_t s, TWI_Bool_t *success);
	void (*inc) (TW_handle_t s);
} TW_Thread_sem_cb_t;

typedef struct TW_Thread_tls_cb_t {
	terr_t (*create) (TW_handle_t *t);
	void (*free) (TW_handle_t t);
	void (*store) (TW_handle_t t, void *data);
	void *(*load) (TW_handle_t t);
} TW_Thread_tls_cb_t;

typedef struct TW_Thread_driver_t {
	terr_t (*init) (int *argc, char **argv[]);
	terr_t (*finalize) (void);

	terr_t (*create) (TWTD_driver_cb_t main, void *data,
					  TW_handle_t *ht);			  // Create thread
	terr_t (*join) (TW_handle_t ht, void **ret);  // Wait for thread
	terr_t (*cancel) (TW_handle_t ht);			  // End a thread
	void (*exit) (void *ret);					  // End calling thread

	TW_Thread_mutex_cb_t mutex;
	TW_Thread_rwlock_cb_t rwlock;
	TW_Thread_sem_cb_t sem;
	TW_Thread_tls_cb_t tls;
} TW_Thread_driver_t;

#ifdef _WIN32
#else
extern TW_Thread_driver_t TWPOSIX_Driver;
#endif