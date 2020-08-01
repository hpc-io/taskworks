/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver engine implementation */

#include "twabt.h"

// Initialize the task engine
/**
 * @brief  Create a new engine
 * @note
 * @param  num_worker: Number of worker threads (ES)
 * @param  evt_driver: The event driver to use
 * @param  *engine: A pointer to the handle of the created engine
 * @retval TW_SUCCESS on success or translated error code on failure
 */
terr_t TWABT_Engine_create (int num_worker, void *dispatcher_obj, TW_Handle_t *engine) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	// ABT_pool_def pool_def;
	// ABT_pool_config pool_config;
	ABT_sched_config_var sched_cfg_var;
	ABT_sched_def sched_def;
	ABT_sched_config sched_cfg;
	TWABT_Engine_t *ep = NULL;

	// Allocate engine structure
	ep = (TWABT_Engine_t *)TWI_Malloc (sizeof (TWABT_Engine_t));
	CHECK_PTR (ep)
	ep->evt_loop	   = NULL;
	ep->schedulers	   = NULL;
	ep->ess			   = NULL;
	ep->ness		   = num_worker;
	ep->ness_alloc	   = ep->ness;
	ep->dispatcher_obj = dispatcher_obj;
	TWI_Nb_list_create (&(ep->tasks));

	// Init event loop
	TWI_Mutex_init (&(ep->evt_lock));
	ep->evt_driver = TWI_Active_evt_driver;
	if (ep->evt_driver) {
		err = ep->evt_driver->Loop_create (&(ep->evt_loop));
		CHECK_ERR
	}

	// Create the task pool
	for (i = 0; i < TWI_TASK_NUM_PRIORITY_LEVEL; i++) {
		abterr =
			ABT_pool_create_basic (ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE, ep->pools + i);
		CHECK_ABTERR
	}

	// Set up schedular definition arg
	sched_def.type			= ABT_SCHED_TYPE_TASK;
	sched_def.init			= TWABTI_Sched_init;
	sched_def.run			= TWABTI_Sched_run;
	sched_def.free			= TWABTI_Sched_finalize;
	sched_def.get_migr_pool = NULL;
	// Optional schedular args
	sched_cfg_var.idx  = 0;
	sched_cfg_var.type = ABT_SCHED_CONFIG_PTR;
	abterr = ABT_sched_config_create (&sched_cfg, sched_cfg_var, ep, ABT_sched_config_var_end);
	CHECK_ABTERR

	// Create the scheduler
	ep->schedulers = (ABT_sched *)TWI_Malloc (sizeof (ABT_sched) * (size_t) (ep->ness_alloc));
	CHECK_PTR (ep->schedulers)
	for (i = 0; i < num_worker; i++) {
		abterr = ABT_sched_create (&sched_def, TWI_TASK_NUM_PRIORITY_LEVEL, ep->pools, sched_cfg,
								   ep->schedulers + i);
		CHECK_ABTERR
	}

	// Create and start ESs, must run last
	ep->ess = (ABT_xstream *)TWI_Malloc (sizeof (ABT_xstream) * (size_t) (ep->ness_alloc));
	CHECK_PTR (ep->ess)
	for (i = 0; i < num_worker; i++) {
		abterr = ABT_xstream_create (ep->schedulers[i], ep->ess + i);
		CHECK_ABTERR
		// API removed in new version?
		// abterr = ABT_xstream_start (ep->ess[i]);
		// CHECK_ABTERR
	}

	// Add to opened engine list
	TWI_Ts_vector_push_back (TWABTI_Engines, ep);

	*engine = ep;

err_out:;
	if (err) {
		if (ep) {
			if (ep->ess) {
				for (i = 0; i < num_worker; i++) {
					ABT_xstream_cancel (ep->ess[i]);
					ABT_xstream_free (ep->ess + i);
				}
				TWI_Free (ep->ess);
			}
			if (ep->schedulers) {
				for (i = 0; i < num_worker; i++) { ABT_sched_free (ep->schedulers + i); }
				TWI_Free (ep->schedulers);
			}
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
terr_t TWABT_Engine_free (TW_Handle_t engine) {
	terr_t err		   = TW_SUCCESS;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;

	err = TWABTI_Engine_free (ep);
	CHECK_ERR

	TWI_Ts_vector_swap_erase (TWABTI_Engines, ep);

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
terr_t TWABT_Engine_do_work (TW_Handle_t engine, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t have_job;
	ttime_t stoptime;
	TWI_Nb_list_itr_t i;
	TWABT_Task_t *tp;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;

	TWI_Nb_list_inc_ref (ep->tasks);
	i		 = TWI_Nb_list_begin (ep->tasks);
	stoptime = TWI_Time_now () + timeout;
	do {
		tp = (TWABT_Task_t *)i->data;

		if (OPA_load_int (&(tp->status)) == TW_Task_STAT_QUEUEING) {
			err = TWABTI_Task_run (tp, &have_job);
			CHECK_ERR
		}

		if (timeout == TW_ONCE || (!have_job)) break;

		i = TWI_Nb_list_next (i);
	} while (TWI_Time_now () < stoptime && have_job);

err_out:;
	TWI_Nb_list_dec_ref (ep->tasks);
	return err;
}

/**
 * @brief  Set the number of workers to num_worker
 * @note
 * @param  engine: Handle to the engine.
 * @param  num_worker: Number of workers
 * @retval
 */
terr_t TWABT_Engine_set_num_workers (TW_Handle_t TWI_UNUSED engine, int TWI_UNUSED num_worker) {
	terr_t err = TW_SUCCESS;
	// int abterr;
	// int i;
	// ABT_pool_def pool_def;
	// ABT_pool_config pool_config;
	// ABT_sched_config_var sched_cfg_var;
	// ABT_sched_def sched_def;
	// ABT_sched_config sched_cfg;
	// TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;

	ASSIGN_ERR (TW_ERR_NOT_SUPPORTED)
	/*
	if (num_worker < ep->ness) {
		for (i = 0; i < num_worker; i++) {
			abterr = ABT_xstream_cancel (ep->ess[i]);
			CHECK_ABTERR
			abterr = ABT_xstream_free (ep->ess + i);
			CHECK_ABTERR
			abterr = ABT_sched_free (ep->schedulers + i);
			CHECK_ABTERR
		}
	} else if (num_worker > ep->ness) {
		if (num_worker > ep->ness_alloc) {}
	}
*/
err_out:;
	return err;
}