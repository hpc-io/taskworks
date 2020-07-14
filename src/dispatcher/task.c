/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file
 * @brief  Task related APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include "dispatcher.h"
/// \endcond

/**
 * @brief  Create a new task
 * @note
 * @param  task_cb: The function to perform the task
 * @param  *task_data: The data used by task_cb
 * @param  dep_cb: The function to decide whether the task is ready to run
 * @param  *dep_data: The data used by dep_cb
 * @param  tag: An integer that can be used by the application for identifiation purpose
 * @param  *task: Handle to the task created
 * @retval
 */
terr_t TW_Task_create (TW_Task_handler_t task_cb,
					   void *task_data,
					   TW_Task_dep_handler_t dep_cb,
					   TW_Task_dep_stat_handler_t dep_stat_cb,
					   int tag,
					   TW_Task_handle_t *task) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t tp;

	tp			= (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	tp->objtype = TW_Obj_type_task;
	tp->driver	= TWI_Active_driver;

	err = tp->driver->Task_create (task_cb, task_data, dep_cb, dep_stat_cb, tag, &(tp->driver_obj));
	CHK_ERR

	*task = tp;

err_out:;
	if (err) { TWI_Free (tp); }
	return err;
}

extern terr_t TW_Task_free (TW_Task_handle_t task) {
	terr_t err = TW_SUCCESS;

	if (task->dep_cb_data) {}

err_out:;
	return err;
}

extern terr_t TW_Task_create_barrier (TW_Engine_handle_t engine,  // Must have option of global
									  int dep_tag,
									  int tag,
									  TW_Task_handle_t taskp);	// Create a new barrier task

// Controls
extern terr_t TW_Task_commit (TW_Task_handle_t task,
							  TW_Engine_handle_t engine);	   // Put the task into the dag
extern terr_t TW_Task_commit_barrier (TW_Task_handle_t task);  // Put the task into the dag
extern terr_t TW_Task_retract (TW_Task_handle_t task);		   // Remove task form the dag

// Wait
extern terr_t TW_Task_wait (TW_Task_handle_t task,
							int64_t timeout);  // Wait for a single task to complete. The
											   // calling thread joins the worker on the
											   // job being waited and all its parents.
extern terr_t TW_Task_wait_ex (TW_Task_handle_t *tasks,
							   int num,
							   int64_t timeout);  // Wait for a multiple task to complete.

// Task dependency API
extern terr_t TW_Task_add_dep (TW_Task_handle_t child, TW_Task_handle_t parent);
extern terr_t TW_Task_rm_dep (TW_Task_handle_t child, TW_Task_handle_t parent);

// Info
extern terr_t TW_Task_get_status (TW_Task_handle_t task, int *statusp);
extern terr_t TW_Task_get_data (TW_Task_handle_t task, void **datap);
extern terr_t TW_Task_get_tag (TW_Task_handle_t task, int *tagp);
