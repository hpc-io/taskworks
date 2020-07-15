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

	err = tp->driver->Task_create (task_cb, task_data, dep_cb, dep_stat_cb, tag, tp,
								   &(tp->driver_obj));
	CHK_ERR

	*task = tp;

err_out:;
	if (err) { TWI_Free (tp); }
	return err;
}

terr_t TW_Task_free (TW_Task_handle_t task) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	err = tp->driver->Task_free (tp->driver_obj);
	CHK_ERR

err_out:;
	return err;
}
// Create a new barrier task
terr_t TW_Task_create_barrier (TW_Engine_handle_t engine,  // Must have option of global
							   int dep_tag,
							   int tag,
							   TW_Task_handle_t *task) {}

// Controls
terr_t TW_Task_commit (TW_Task_handle_t task, TW_Engine_handle_t engine) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;
	TW_Obj_handle_t ep = (TW_Obj_handle_t)engine;

	err = tp->driver->Task_commit (tp->driver_obj, ep->driver_obj);
	CHK_ERR

err_out:;
	return err;
}

terr_t TW_Task_commit_barrier (TW_Task_handle_t task);	// Put the task into the dag
terr_t TW_Task_retract (TW_Task_handle_t task) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;
	err				   = tp->driver->Task_retract (tp->driver_obj);
	CHK_ERR

err_out:;
	return err;
}

// Wait for a single task to complete. The
// calling thread joins the worker on the
// job being waited and all its parents.
terr_t TW_Task_wait (TW_Task_handle_t task, int64_t timeout) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	err = tp->driver->Task_wait_single (tp->driver_obj, timeout);
	CHK_ERR

err_out:;
	return err;
}
terr_t TW_Task_wait_multi (TW_Task_handle_t *tasks, int num, int64_t timeout) {
	terr_t err = TW_SUCCESS;
	int i;
	ttime_t tstop, tnow;
	TW_Obj_handle_t tp;
	TW_Handle_t *driver_objs = NULL;

	driver_objs = (TW_Handle_t *)TWI_Malloc (sizeof (TW_Handle_t) * num);
	CHK_PTR (driver_objs);

	err = tp->driver->Task_wait (driver_objs, num, timeout);
	CHK_ERR

err_out:;
	TWI_Free (driver_objs);
	return err;
}

// Task dependency API
terr_t TW_Task_add_dep (TW_Task_handle_t child, TW_Task_handle_t parent) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t cp = (TW_Obj_handle_t)child;
	TW_Obj_handle_t pp = (TW_Obj_handle_t)parent;

	err = cp->driver->Task_add_dep (cp->driver_obj, pp->driver_obj);
	CHK_ERR

err_out:;
	return err;
}

terr_t TW_Task_rm_dep (TW_Task_handle_t child, TW_Task_handle_t parent) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t cp = (TW_Obj_handle_t)child;
	TW_Obj_handle_t pp = (TW_Obj_handle_t)parent;

	err = cp->driver->Task_rm_dep (cp->driver_obj, pp->driver_obj);
	CHK_ERR

err_out:;
	return err;
}

// Info
terr_t TW_Task_get_status (TW_Task_handle_t task, int *statusp) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_status, statusp);
	CHK_ERR

err_out:;
	return err;
}
terr_t TW_Task_get_data (TW_Task_handle_t task, void **datap) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_data, datap);
	CHK_ERR

err_out:;
	return err;
}
terr_t TW_Task_get_tag (TW_Task_handle_t task, int *tagp) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_tag, tagp);
	CHK_ERR

err_out:;
	return err;
}
