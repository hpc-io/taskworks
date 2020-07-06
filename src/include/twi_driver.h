/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Driver interface */

#pragma once

#include <opa_primitives.h>

#include "taskworks.h"

typedef enum TW_Task_inq_type_t {
	TW_Task_inq_type_status,
} TW_Task_inq_type_t;

typedef struct TW_Driver_t {
	/* Engine callbacks */
	terr_t (*Engine_create) (int num_worker,
							 TW_Event_driver_handle_t evt_driver,
							 TW_Engine_handle_t *heng);	 // Initialize the task engine
														 // with num_worker workers
	terr_t (*Engine_free) (TW_Engine_handle_t heng);	 // Finalize the task engine
	terr_t (*Engine_do_work) (TW_Engine_handle_t heng,
							  ttime_t timeout);	 // Run a single task using the calling thread
	terr_t (*Engine_set_num_workers) (
		TW_Engine_handle_t heng,
		int num_worker);  // Increase the number of worker by num_worker

	/* Task callbacks */
	terr_t (*Task_create) (TW_Task_handler_t task_cb,
						   void *task_data,
						   TW_Task_dep_handler_t dep_cb,
						   int tag,
						   TW_Task_handle_t *htask);  // Create a new task

	terr_t (*Task_free) (TW_Task_handle_t htask);			 // Free up a task
	terr_t (*Task_create_barrier) (TW_Engine_handle_t heng,	 // Must have option of global
								   int dep_tag,
								   int tag,
								   TW_Task_handle_t *htask);  // Create a new barrier task
	terr_t (*Task_commit) (TW_Task_handle_t htask,
						   TW_Engine_handle_t heng);	  // Put the task into the dag
	terr_t (*Task_retract) (TW_Task_handle_t htask);	  // Remove task form the dag
	terr_t (*Task_wait_single) (TW_Task_handle_t htask);  // Wait for a single task to complete. The
														  // calling thread joins the worker on the
														  // job being waited and all its parents.
	terr_t (*Task_wait) (TW_Task_handle_t *htasks,
						 int *num_tasks,
						 ttime_t timeout);	// Wait for a multiple task to complete.
	terr_t (*Task_add_dep) (TW_Task_handle_t child, TW_Task_handle_t parent);
	terr_t (*Task_rm_dep) (TW_Task_handle_t child, TW_Task_handle_t parent);
	terr_t (*Task_inq) (TW_Task_handle_t htask, TW_Task_inq_type_t inqtype, int *ret);

	/* Event callbacks */
	terr_t (*Event_create) (TW_Event_handler_t evt_cb,
							void *evt_data,
							TW_Event_attr_t attr,
							TW_Event_handle_t *hevt);  // Create a new event
	terr_t (*Event_free) (TW_Event_handle_t hevt);
	terr_t (*Event_commit) (TW_Engine_handle_t heng,
							TW_Event_handle_t hevt);   // Commit event, start watching
	terr_t (*Event_retract) (TW_Event_handle_t hevt);  // Stop watching
} TW_Driver_t;