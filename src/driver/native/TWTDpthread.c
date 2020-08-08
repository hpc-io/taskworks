/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* TaskWroks thread driver based on pthread*/

#include "TWI.h"
#include "TWTD.h"
#include "pthread.h"

terr_t tw_pthread_driver_init (TWTD_driver_cb_t main,
							   int N,
							   TW_handle_t *ht);	 // Initialize the driver with N threads
terr_t tw_pthread_driver_finalize (TW_handle_t ht);	 // Finalize the engine with N threads

terr_t tw_pthread_driver_add_thread (TW_handle_t ht, int N);  // Increase the number of threads by N
terr_t tw_pthread_driver_end_thread (TW_handle_t ht, int N);  // End current thread

terr_t tw_pthread_driver_create_mutex (TW_handle_t *ht);  // Initialize a mutex
terr_t tw_pthread_driver_free_mutex (TW_handle_t ht);	  // Free a mutex

terr_t tw_pthread_driver_create_semaphore (TW_handle_t *ht);  // Initialize a semaphore
terr_t tw_pthread_driver_free_semaphore (TW_handle_t ht);	  // Free a semaphore

terr_t tw_pthread_driver_create_rw_lock (TW_handle_t *ht);	// Initialize a read/write lock
terr_t tw_pthread_driver_free_rw_lock (TW_handle_t ht);		// Free a read/write lock

terr_t tw_pthread_driver_create_condition_var (
	TW_handle_t *ht);										   // Initialize a conditional variable
terr_t tw_pthread_driver_free_condition_var (TW_handle_t ht);  // Free a conditional variable

TWTD_driver_t TWTD_pthread = {tw_pthread_driver_init,
							  tw_pthread_driver_finalize,
							  tw_pthread_driver_add_thread,
							  tw_pthread_driver_end_thread,
							  tw_pthread_driver_create_mutex,
							  tw_pthread_driver_free_mutex,
							  tw_pthread_driver_create_semaphore,
							  tw_pthread_driver_free_semaphore,
							  tw_pthread_driver_create_rw_lock,
							  tw_pthread_driver_free_rw_lock,
							  tw_pthread_driver_create_condition_var,
							  tw_pthread_driver_free_condition_var};

typedef struct TWTD_pthread_thread_context {
	pthread_t handle;
	TWTD_driver_cb_t main_func;
	int idx;
} TWTD_pthread_thread_context;

typedef struct TWTD_pthread_driver_context {
	int n_threads;
	int n_threads_alloc;
	TWTD_pthread_thread_context *threads;

} TWTD_pthread_driver_context;

void *TWTD_pthread_driver_cb_fn (void *dp) {}

/**
 * Initialize pthread thread driver with N threads
 *
 * Allocate and initialize thethread driver structure
 * Create N threads and store the handle into an array
 */
terr_t tw_pthread_driver_init (TWTD_driver_cb_t main, int N, TW_handle_t *ht) {
	terr_t err = TW_SUCCESS;
	int ret;
	int i, j;
	TWTD_pthread_driver_context *ctxp;

	ctxp = TWI_malloc (sizeof (TWTD_pthread_driver_context));
	TWE_CHECK_PTR (ctxp)

	ctxp->n_threads		  = N;
	ctxp->n_threads_alloc = N;
	ctxp->threads = TWI_malloc (sizeof (TWTD_pthread_thread_context) * ctxp->n_threads_alloc);
	TWE_CHECK_PTR (ctxp->threads)

	for (i = 0; i < N; i++) {
		ctxp->threads[i].idx	   = i;
		ctxp->threads[i].main_func = main;
		ctxp->threads[i];
		ret =
			pthread_create (ctxp->threads + i, NULL, TWTD_pthread_driver_cb_fn, ctxp->threads + i);
		if (ret != 0) { err = TWE_THREAD_CREATE; }
	}
	*ht = ctxp;
err_out:;
	return err;
}
