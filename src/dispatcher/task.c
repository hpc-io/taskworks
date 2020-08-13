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
 * @param  priority:
 * @param  *task: Handle to the task created
 * @retval
 */
terr_t TW_Task_create_ex (TW_Task_handler_t task_cb,
						  void *task_data,
						  TW_Task_dep_handler_t dep_handler,
						  int tag,
						  int priority,
						  TW_Task_status_handler_t stat_handler,
						  int status_mask,
						  TW_Task_handle_t *task) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t tp;

	if (tag < 0) ASSIGN_ERR (TW_ERR_INVAL)
	if (priority < 0 || priority > TW_TASK_PRIORITY_STANDARD) ASSIGN_ERR (TW_ERR_INVAL)

	tp = (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	CHECK_PTR (tp)
	tp->objtype = TW_Obj_type_task;
	tp->driver	= TWI_Active_driver;

	err = tp->driver->Task_create (task_cb, task_data, dep_handler, tag, priority, stat_handler,
								   status_mask, tp, &(tp->driver_obj));
	CHECK_ERR

	*task = tp;

err_out:;
	if (err) { TWI_Free (tp); }
	return err;
}

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
					   TW_Task_dep_handler_t dep_handler,
					   int tag,
					   TW_Task_handle_t *task) {
	return TW_Task_create_ex (task_cb, task_data, dep_handler, tag, TW_TASK_PRIORITY_STANDARD, NULL,
							  0, task);
}

terr_t TW_Task_free (TW_Task_handle_t task) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_free (tp->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

// Create a new barrier task
terr_t TW_Task_create_barrier (TW_Engine_handle_t engine,  // Must have option of global
							   int dep_tag,
							   int tag,
							   TW_Task_handle_t *task) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t tp;

	if (engine && engine->objtype != TW_Obj_type_engine) ASSIGN_ERR (TW_ERR_INVAL)
	if (tag < 0) ASSIGN_ERR (TW_ERR_INVAL)

	tp = (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	CHECK_PTR (tp)
	tp->objtype = TW_Obj_type_task;
	tp->driver	= TWI_Active_driver;

	err = tp->driver->Task_create_barrier (engine, dep_tag, tag, tp, &(tp->driver_obj));
	CHECK_ERR

	*task = tp;

err_out:;
	if (err) { TWI_Free (tp); }
	return err;
}

// Controls
terr_t TW_Task_commit (TW_Task_handle_t task, TW_Engine_handle_t engine) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;
	TW_Obj_handle_t ep = (TW_Obj_handle_t)engine;

	CHK_HANDLE (task, TW_Obj_type_task)
	CHK_HANDLE (engine, TW_Obj_type_engine)

	err = tp->driver->Task_commit (tp->driver_obj, ep->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

terr_t TW_Task_commit_barrier (TW_Task_handle_t task) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_commit (tp->driver_obj, NULL);
	CHECK_ERR

err_out:;
	return err;
}

terr_t TW_Task_retract (TW_Task_handle_t task) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_retract (tp->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

// Wait for a single task to complete. The
// calling thread joins the worker on the
// job being waited and all its parents.
terr_t TW_Task_wait (TW_Task_handle_t task, int64_t timeout) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_wait_single (tp->driver_obj, timeout);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Task_wait_multi (TW_Task_handle_t *tasks, int num, int64_t timeout) {
	terr_t err = TW_SUCCESS;
	int i;
	TW_Handle_t *driver_objs = NULL;

	if (num) {
		driver_objs = (TW_Handle_t *)TWI_Malloc (sizeof (TW_Handle_t) * (size_t)num);
		CHECK_PTR (driver_objs);

		for (i = 0; i < num; i++) {
			CHK_HANDLE (tasks[i], TW_Obj_type_task)
			driver_objs[i] = ((TW_Obj_handle_t)tasks[i])->driver_obj;
		}
		err = ((TW_Obj_handle_t)tasks[0])->driver->Task_wait (driver_objs, num, timeout);
		CHECK_ERR
	}

err_out:;
	TWI_Free (driver_objs);
	return err;
}

// Task dependency API
terr_t TW_Task_add_dep (TW_Task_handle_t child, TW_Task_handle_t parent) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t cp = (TW_Obj_handle_t)child;
	TW_Obj_handle_t pp = (TW_Obj_handle_t)parent;

	CHK_HANDLE (child, TW_Obj_type_task)
	CHK_HANDLE (parent, TW_Obj_type_task)

	err = cp->driver->Task_add_dep (cp->driver_obj, pp->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

terr_t TW_Task_rm_dep (TW_Task_handle_t child, TW_Task_handle_t parent) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t cp = (TW_Obj_handle_t)child;
	TW_Obj_handle_t pp = (TW_Obj_handle_t)parent;

	CHK_HANDLE (child, TW_Obj_type_task)
	CHK_HANDLE (parent, TW_Obj_type_task)

	err = cp->driver->Task_rm_dep (cp->driver_obj, pp->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

// Info
terr_t TW_Task_get_status (TW_Task_handle_t task, int *statusp) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_status, statusp);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Task_get_data (TW_Task_handle_t task, void **datap) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_data, datap);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Task_get_tag (TW_Task_handle_t task, int *tagp) {
	terr_t err		   = TW_SUCCESS;
	TW_Obj_handle_t tp = (TW_Obj_handle_t)task;

	CHK_HANDLE (task, TW_Obj_type_task)

	err = tp->driver->Task_inq (tp->driver_obj, TW_Task_inq_type_tag, tagp);
	CHECK_ERR

err_out:;
	return err;
}

const char *TW_Task_status_str (int status) {
	if (status & TW_TASK_STAT_IDLE) {
		return "idle";
	} else if (status & TW_TASK_STAT_DEPHOLD) {
		return "dep. hold";
	} else if (status & TW_TASK_STAT_READY) {
		return "ready";
	} else if (status & TW_TASK_STAT_QUEUE) {
		return "queuing";
	} else if (status & TW_TASK_STAT_RUNNING) {
		return "running";
	} else if (status & TW_TASK_STAT_COMPLETED) {
		return "completed";
	} else if (status & TW_TASK_STAT_ABORTED) {
		return "aborted";
	} else if (status & TW_TASK_STAT_FAILED) {
		return "failed";
	} else if (status & TW_TASK_STAT_FINAL) {
		return "finalizing";
	} else if (status & TW_TASK_STAT_TRANS) {
		return "transition";
	}

	return "unknown";
}