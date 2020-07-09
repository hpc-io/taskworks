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

typedef struct TWABT_Engine_t {
	int ness;				// Number of threads (ES)
	int ness_alloc;			// Size of schedulers and ess
	ABT_pool pool;			// Task pool of th engine
	ABT_sched *schedulers;	// Task scheduler
	ABT_xstream *ess;		// Threads (ES)
} TWABT_Engine_t;

// Initialize the task engine
/**
 * @brief  Create a new engine
 * @note
 * @param  num_worker: Number of worker threads (ES)
 * @param  evt_driver: The event driver to use
 * @param  *engine: A pointer to the handle of the created engine
 * @retval TW_SUCCESS on success or translated error code on failure
 */
terr_t TWABT_Engine_create (int num_worker, TW_Handle_t *engine) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	// ABT_pool_def pool_def;
	// ABT_pool_config pool_config;
	// ABT_sched_config_var sched_cfg_var;
	ABT_sched_def sched_def;
	ABT_sched_config sched_cfg;
	TWABT_Engine_t *ep = NULL;

	// Allocate engine structure
	ep = (TWABT_Engine_t *)TWI_Malloc (sizeof (TWABT_Engine_t));
	CHK_PTR (ep)
	ep->schedulers = NULL;
	ep->ess		   = NULL;
	ep->ness	   = num_worker;
	ep->ness_alloc = ep->ness;

	// Create the task pool
	abterr = ABT_pool_create_basic (ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE, &(ep->pool));
	CHECK_ABTERR

	// Set up schedular definition arg
	sched_def.type			= ABT_SCHED_TYPE_TASK;
	sched_def.init			= TWABTI_Sched_init;
	sched_def.run			= TWABTI_Sched_run;
	sched_def.free			= TWABTI_Sched_finalize;
	sched_def.get_migr_pool = NULL;
	// Optional schedular args
	//// sched_cfg_var.idx  = 0;
	//// sched_cfg_var.type = ABT_SCHED_CONFIG_INT;
	//// abterr=ABT_sched_config_create (&sched_cfg, ABT_sched_config_var_end);

	// Create the scheduler
	ep->schedulers = (ABT_sched *)TWI_Malloc (sizeof (ABT_sched) * ep->ness_alloc);
	CHK_PTR (ep->schedulers)
	for (i = 0; i < num_worker; i++) {
		abterr = ABT_sched_create (&sched_def, 1, &(ep->pool), ABT_SCHED_CONFIG_NULL,
								   ep->schedulers + i);
		CHECK_ABTERR
	}

	// Create and start ESs
	ep->ess = (ABT_xstream *)TWI_Malloc (sizeof (ABT_xstream) * ep->ness_alloc);
	CHK_PTR (ep->ess)
	for (i = 0; i < num_worker; i++) {
		abterr = ABT_xstream_create (ep->schedulers[i], ep->ess + i);
		CHECK_ABTERR
		// API removed in new version?
		// abterr = ABT_xstream_start (ep->ess[i]);
		// CHECK_ABTERR
	}

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
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;

	// Stop and free the ESs
	for (i = 0; i < ep->ness; i++) {
		abterr = ABT_xstream_join (ep->ess[i]);
		CHECK_ABTERR
		abterr = ABT_xstream_free (ep->ess + i);
		CHECK_ABTERR
	}
	TWI_Free (ep->ess);

	// Free the schedulars
	for (i = 0; i < ep->ness; i++) {
		abterr = ABT_sched_free (ep->schedulers + i);
		CHECK_ABTERR
	}
	TWI_Free (ep->schedulers);

	// Pool freed automatically when all schedulers get freed
	////abterr = ABT_pool_free (&(ep->pool));

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

	RET_ERR (TW_ERR_NOT_SUPPORTED)

err_out:;
	return err;
}

/**
 * @brief  Set the number of workers to num_worker
 * @note
 * @param  engine: Handle to the engine.
 * @param  num_worker: Number of workers
 * @retval
 */
terr_t TWABT_Engine_set_num_workers (TW_Handle_t engine, int num_worker) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	// ABT_pool_def pool_def;
	// ABT_pool_config pool_config;
	// ABT_sched_config_var sched_cfg_var;
	ABT_sched_def sched_def;
	ABT_sched_config sched_cfg;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;

	RET_ERR (TW_ERR_NOT_SUPPORTED)

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

err_out:;
	return err;
}