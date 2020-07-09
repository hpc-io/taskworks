/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Task APIs */

#pragma once

#include <opa_primitives.h>
#include <stdint.h>

#include "taskworks.h"

typedef struct TW_Obj_t *TW_Task_handle_t;

/* Task status
 * We do not use enum because it is not supported by openpa
 */
// The task hasn't been commited, user can modify the task
#define TW_Task_STAT_PENDING 1
#define TW_Task_STAT_WAITING 2	// Commited in the flow graph, waiting on job dependency
// Ready for processing. Waiting in the job queue for available worker
#define TW_Task_STAT_QUEUEING  3
#define TW_Task_STAT_RUNNING   4  // A worker is handling the job
#define TW_Task_STAT_COMPLETWD 5  // The job is completed
// The dependency can never be satisfied (eg. parent job failed). The job is removed from the flow
// graph
#define TW_Task_STAT_ABORTWD 6

/* Callback functions */
typedef terr_t (*TW_Task_handler_t) (void *data);
typedef int (*TW_Task_dep_handler_t) (int num_deps, TW_Task_handle_t *deps, int idx, void **datap);

// Create, free
extern terr_t TW_Task_create (TW_Engine_handle_t engine,
							  TW_Task_handler_t task_cb,
							  void *task_data,
							  TW_Task_dep_handler_t dep_cb,
							  int tag,
							  TW_Task_handle_t *task);	// Create a new task

extern terr_t TW_Task_free (TW_Task_handle_t task);	 // Free up a task

extern terr_t TW_Task_create_barrier (TW_Engine_handle_t engine,  // Must have option of global
									  int dep_tag,
									  int tag,
									  TW_Task_handle_t taskp);	// Create a new barrier task

// Controls
extern terr_t TW_Task_commit (TW_Task_handle_t task);	// Put the task into the dag
extern terr_t TW_Task_retract (TW_Task_handle_t task);	// Remove task form the dag

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
