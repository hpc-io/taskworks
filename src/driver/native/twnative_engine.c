/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Native driver engine implementation */

#include "twnative.h"

// Initialize the task engine
/**
 * @brief  Create a new engine
 * @note
 * @param  num_worker: Number of worker threads (ES)
 * @param  evt_driver: The event driver to use
 * @param  *engine: A pointer to the handle of the created engine
 * @retval TW_SUCCESS on success or translated error code on failure
 */
terr_t TWNATIVE_Engine_create (int num_worker, void *dispatcher_obj, TW_Handle_t *engine) {
	terr_t err = TW_SUCCESS;
	int i;
	TWNATIVE_Thread_arg_t *ta;
	TWNATIVE_Engine_t *ep = NULL;
	DEBUG
	// Allocate engine structure
	ep = (TWNATIVE_Engine_t *)TWI_Malloc (sizeof (TWNATIVE_Engine_t));
	CHECK_PTR (ep)

	// TODO: Select thread driver based on input
	ep->driver = TWNATIVEI_Driver;

	// Init members
	err = ep->driver->mutex.create (&(ep->lock));
	CHECK_ERR
	err = ep->driver->sem.create (&(ep->njob));
	CHECK_ERR
	ep->evt_loop = NULL;
	ep->threads	 = NULL;
	ep->nt		 = num_worker;
	OPA_store_int (&(ep->cur_nt), ep->nt);
	OPA_store_int (&(ep->sleep_nt), 0);
	ep->nt_alloc	   = ep->nt;
	ep->dispatcher_obj = dispatcher_obj;

	// Init event loop
	TWI_Mutex_init (&(ep->evt_lock));
	ep->evt_driver = TWI_Active_evt_driver;
	if (ep->evt_driver) {
		err = ep->evt_driver->Loop_create (&(ep->evt_loop));
		CHECK_ERR
		ep->driver->sem.inc (ep->njob);	 // Release one thread to check for events
	}

	// Init job queue
	for (i = 0; i < TWI_TASK_NUM_PRIORITY_LEVEL; i++) {
		err = TWI_Nb_queue_create (&(ep->queue[i]));
		CHECK_ERR
	}

	// Init task list
	// ep->tasks=TWI_Ts_vector_create();

	// Create and start threads, must run last
	ep->threads = (TW_Handle_t *)TWI_Malloc (sizeof (TW_Handle_t) * (size_t) (ep->nt_alloc));
	CHECK_PTR (ep->threads)
	for (i = 0; i < num_worker; i++) {
		ta = (TWNATIVE_Thread_arg_t *)TWI_Malloc (sizeof (TWNATIVE_Thread_arg_t));
		CHECK_PTR (ta)
		ta->id = i;
		ta->ep = ep;
		err	   = ep->driver->create (TWNATIVE_Engine_scheduler, ta, ep->threads + i);
		CHECK_ERR
	}

	// Add to opened engine list
	TWI_Ts_vector_push_back (TWNATIVEI_Engines, ep);

	*engine = ep;

err_out:;
	if (err) {
		if (ep) {
			if (ep->threads) {
				for (i = 0; i < num_worker; i++) { ep->driver->cancel (ep->threads[i]); }
				TWI_Free (ep->threads);
			}
			// TWI_Ts_vector_free(ep->tasks);
			if (ep->evt_loop) { ep->evt_driver->Loop_free (ep->evt_loop); }
			TWI_Free (ep);
		}
	}

	return err;
}

/**
 * @brief  Free the engine and release all resources
 * @note
 * @param  engine: Handle to the created engine
 * @retval TW_SUCCESS on success or translated error code on failure
 */
terr_t TWNATIVE_Engine_free (TW_Handle_t engine) {
	terr_t err			  = TW_SUCCESS;
	TWNATIVE_Engine_t *ep = (TWNATIVE_Engine_t *)engine;

	err = TWNATIVE_Enginei_free (ep);
	CHECK_ERR

	TWI_Ts_vector_swap_erase (TWNATIVEI_Engines, ep);

err_out:;
	return err;
}

/**
 * @brief Run a single task using the calling thread
 * @note
 * @param  engine: Handle to the engine.
 * @param  timeout: Time commited to perform works. If give 0 or negative, it will return after
 * processing one task or events
 * @retval
 */
terr_t TWNATIVE_Engine_do_work (TW_Handle_t engine, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	ttime_t stoptime;
	TWI_Bool_t have_task;

	TWNATIVE_Engine_t *ep = (TWNATIVE_Engine_t *)engine;
DEBUG
	stoptime = TWI_Time_now () + timeout;

	TWI_Disposer_join (TWNATIVEI_Disposer);
	DEBUG
	if (timeout == TW_ONCE) {DEBUG
		err = TWNATIVE_Engine_scheduler_core (ep, &have_task);
		CHECK_ERR
	} else {DEBUG
		if (timeout == TW_INFINITE) stoptime = INT64_MAX;
		do {DEBUG
			err = TWNATIVE_Engine_scheduler_core (ep, &have_task);
			CHECK_ERR
		} while (have_task && (TWI_Time_now () < stoptime));
	}
	DEBUG
err_out:;
	TWI_Disposer_leave (TWNATIVEI_Disposer);
	return err;
}

/**
 * @brief  Set the number of workers to num_worker
 * @note
 * @param  engine: Handle to the engine.
 * @param  num_worker: Number of workers
 * @retval
 */
terr_t TWNATIVE_Engine_set_num_workers (TW_Handle_t TWI_UNUSED engine, int TWI_UNUSED num_worker) {
	terr_t err = TW_SUCCESS;
	int i;
	int old_nt;
	TW_Handle_t tmp;
	TWNATIVE_Thread_arg_t *ta;
	TWNATIVE_Engine_t *ep = (TWNATIVE_Engine_t *)engine;
	DEBUG
	// Write for prev adjustment to finish
	while (ep->nt != OPA_load_int (&(ep->cur_nt)))
		;

	// Enlarge thread array
	if (num_worker > ep->nt_alloc) {
		do { ep->nt_alloc <<= 1; } while (num_worker > ep->nt_alloc);
		ep->driver->mutex.lock (ep->lock);
		tmp = TWI_Realloc (ep->threads, sizeof (TW_Handle_t) * (size_t)ep->nt_alloc);
		CHECK_PTR (tmp)
		ep->threads = tmp;
		ep->driver->mutex.unlock (ep->lock);
	}

	// Set current thread target
	old_nt = ep->nt;
	ep->nt = num_worker;

	// Create new thread if increased
	ep->driver->mutex.lock (ep->lock);
	for (i = old_nt; i < num_worker; i++) {
		ta = (TWNATIVE_Thread_arg_t *)TWI_Malloc (sizeof (TWNATIVE_Thread_arg_t));
		CHECK_PTR (ta)
		ta->id = i;
		ta->ep = ep;
		err	   = ep->driver->create (TWNATIVE_Engine_scheduler, ta, ep->threads + i);
		CHECK_ERR
	}
	ep->driver->mutex.unlock (ep->lock);

err_out:;
	return err;
}
