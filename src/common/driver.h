/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Driver interface */

#pragma once

#include "taskworks_internal.h"
#include "twi_time.h"

typedef struct TW_Driver_t *TW_Driver_handle_t;
typedef struct TW_Event_driver_t *TW_Event_driver_handle_t;

extern TW_Event_driver_handle_t evt_driver;
extern TW_Driver_handle_t driver;

typedef enum TW_Task_inq_type_t {
	TW_Task_inq_type_status,
} TW_Task_inq_type_t;

typedef struct TW_Driver_t {
	/* Init */
	terr_t (*Init) (int *argc,
					char ***argv);	// Iinitialize the driver
	terr_t (*Finalize) ();			// Finalize the driver

	/* Engine callbacks */
	terr_t (*Engine_create) (int num_worker,
							 TW_Handle_t *engine);	// Initialize the task engine
													// with num_worker workers
	terr_t (*Engine_free) (TW_Handle_t engine);		// Finalize the task engine
	terr_t (*Engine_do_work) (TW_Handle_t engine,
							  ttime_t timeout);	 // Run a single task using the calling thread
	terr_t (*Engine_set_num_workers) (
		TW_Handle_t engine,
		int num_worker);  // Increase the number of worker by num_worker

	/* Task callbacks */
	terr_t (*Task_create) (TW_Task_handler_t task_cb,
						   void *task_data,
						   TW_Task_dep_handler_t dep_cb,
						   TW_Task_dep_stat_handler_t dep_stat_cb,
						   int tag,
						   TW_Handle_t *htask);	 // Create a new task

	terr_t (*Task_free) (TW_Handle_t htask);			// Free up a task
	terr_t (*Task_create_barrier) (TW_Handle_t engine,	// Must have option of global
								   int dep_tag,
								   int tag,
								   TW_Handle_t *htask);	 // Create a new barrier task
	terr_t (*Task_commit) (TW_Handle_t htask,
						   TW_Handle_t engine);		 // Put the task into the dag
	terr_t (*Task_retract) (TW_Handle_t htask);		 // Remove task form the dag
	terr_t (*Task_wait_single) (TW_Handle_t htask);	 // Wait for a single task to complete. The
													 // calling thread joins the worker on the
													 // job being waited and all its parents.
	terr_t (*Task_wait) (TW_Handle_t *htasks,
						 int *num_tasks,
						 ttime_t timeout);	// Wait for a multiple task to complete.
	terr_t (*Task_add_dep) (TW_Handle_t child, TW_Handle_t parent);
	terr_t (*Task_rm_dep) (TW_Handle_t child, TW_Handle_t parent);
	terr_t (*Task_inq) (TW_Handle_t htask, TW_Task_inq_type_t inqtype, int *ret);

	/* Event callbacks */
	terr_t (*Event_create) (TW_Event_handler_t evt_cb,
							void *evt_data,
							TW_Event_attr_t attr,
							TW_Handle_t *hevt);	 // Create a new event
	terr_t (*Event_free) (TW_Handle_t hevt);
	terr_t (*Event_commit) (TW_Handle_t engine,
							TW_Handle_t hevt);	 // Commit event, start watching
	terr_t (*Event_retract) (TW_Handle_t hevt);	 // Stop watching
} TW_Driver_t;

#ifdef ENABLE_ABT
extern TW_Driver_t TWABT_Driver;
#endif