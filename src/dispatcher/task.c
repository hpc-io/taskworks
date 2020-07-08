/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* User level Task API */

#include "taskworks_internal.h"

typedef struct TW_task {
	OPA_int_t status;  // Status of the task

	TW_Task_handler_t cb;  // Function to run
	void *cb_data;		   // Input parameters to the task function
	terr_t ret;			   // Return value of the task function, only meanningfull when
				 // stat is TW_Task_stat_completed # use void

	TW_Task_dep_handler_t dep_cb;  // Function that decide the status of the task based on
								   // dependency
	void *dep_cb_data;			   // Input parameters to the dependency function function

	// TODO: There must be defualt hard-coded dep handler that takes constant time

	int tag;  // Optional tag to be used by the application

	TW_Engine_handle_t engine;	// Engine handling the task
	void *engine_data;			// Engine specific data
} TW_task;

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
					   void *dep_data,
					   int tag,
					   TW_Task_handle_t *task) {
	terr_t err = TW_SUCCESS;
	TW_Task_handle_t tp;

	tp = (TW_Task_handle_t)TWI_Malloc (sizeof (TW_task));
	CHK_PTR (tp)
	tp->cb		= task_cb;
	tp->cb_data = task_data;
	tp->dep_cb	= dep_cb;
	tp->tag		= tag;

	tp->engine		= NULL;
	tp->engine_data = NULL;

err_out:;
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
