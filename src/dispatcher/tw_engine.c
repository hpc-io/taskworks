/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file
 * @brief  Engine related APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include "dispatcher.h"
/// \endcond

/**
 * @brief  Create a new engine
 * @note
 * @param  num_worker: Number of worker threads int he engine
 * @param  engine_driver: Engine backend
 * @param  evt_driver: Event backend
 * @param  *engine: Handle to engine
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_create (int num_worker, TW_Engine_handle_t *engine) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t ep;

	ep			= (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	ep->objtype = TW_Obj_type_engine;
	ep->driver	= TWI_Active_driver;

	err = ep->driver->Engine_create (num_worker, ep, &(ep->driver_obj));
	CHECK_ERR

	*engine = ep;

err_out:;
	if (err) { TWI_Free (ep); }
	return err;
}

/**
 * @brief Stop the engine and free all associated resource
 * @note
 * @param  engine: Handle to the engine
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_free (TW_Engine_handle_t engine) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (engine, TW_Obj_type_engine)

	err = engine->driver->Engine_free (engine->driver_obj);
	CHECK_ERR

	TWI_Free (engine);

err_out:;
	return err;
}

/**
 * @brief Run a single task using the calling thread
 * @note
 * @param  engine: Handle to the engine
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_progress (TW_Engine_handle_t engine) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (engine, TW_Obj_type_engine)
DEBUG
	err = engine->driver->Engine_do_work (engine->driver_obj, TW_ONCE);

err_out:;

	return err;
}

/**
 * @brief Run a single task using the calling thread for a specific time
 * @note Timeout is checked only after completion of tasks
 * @param  engine: Handle to the engine
 * @param  timeout: Time in micro-second
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_progress_until (TW_Engine_handle_t engine, int64_t timeout) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (engine, TW_Obj_type_engine)
DEBUG
	err = engine->driver->Engine_do_work (engine->driver_obj, (ttime_t)timeout);

err_out:;

	return err;
}

/**
 * @brief  Set the number of worker in an engine
 * @note   Not supproted by all backends
 * @param  engine: Handle to the engine
 * @param  num_worker: Number of workers
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_set_num_worker (TW_Engine_handle_t engine, int num_worker) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (engine, TW_Obj_type_engine)

	err = engine->driver->Engine_set_num_workers (engine->driver_obj, num_worker);

err_out:;

	return err;
}
