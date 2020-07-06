/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* User level engine API */

#include "taskworks_internal.h"

typedef struct TW_Engine_t {
	int num_workers;
#ifdef ENABLE_PARALLEL
	MPI_Comm comm;
	MPI_Info info;
#endif

} TW_Engine_t;

/**
 * @brief  Create a new engine
 * @note
 * @param  num_worker: Number of worker threads int he engine
 * @param  work_driver: Engine backend
 * @param  evt_driver: Event backend
 * @param  *hpool: Handle to engine
 * @retval TW_ERR_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_create (int num_worker,
						 TW_Driver_handle_t work_driver,
						 TW_Event_driver_handle_t evt_driver,
						 TW_Engine_handle_t *hpool) {
	terr_t err;
	int i, j;
}

/**
 * @brief Stop the task engine and finalize all resource
 * @note
 * @param  hpool: Handle to the engine
 * @retval TW_ERR_SUCCESS on success or error code on failure
 */
terr_t TW_Engine_free (TW_Engine_handle_t hpool) {}

/**
 * @brief
 * @note
 * @param  hpool:
 * @retval
 */
terr_t TW_Engine_progress (TW_Engine_handle_t hpool) {
}  // Run a single task using the calling thread

// Draw the task schedular loop as figure figram

// Control by time, running until exhust the time given?
terr_t TW_Engine_progress_until (int time, TW_Engine_handle_t hpool) {
}  // Run a single task using the calling thread

terr_t TW_Engine_add_worker (int num_worker, TW_Engine_handle_t hpool) {
}  // Increase the number of worker by num_worker

terr_t TW_Engine_rm_worker (int num_worker, TW_Engine_handle_t hpool) {
}  // Decrease the number of worker by num_worker
