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

#include <stdint.h>

#include "taskworks.h"

typedef struct TW_Obj_t *TW_Task_handle_t;

/* Task status
 * We do not use enum because it is not supported by openpa
 * Note: The value has meaning and cannot be reassigned arbitrarily. The status can only increase
 * during a single commit session.
 */
#define TW_TASK_STAT_INVAL	   0x0
#define TW_TASK_STAT_IDLE	   0x1	 // The task hasn't been commited, user can modify the task
#define TW_TASK_STAT_DEPHOLD   0x2	 // Commited in the flow graph, waiting on job dependency
#define TW_TASK_STAT_READY	   0x4	 // Ready for processing.
#define TW_TASK_STAT_QUEUE	   0x8	 // Waiting in the job queue for available worker
#define TW_TASK_STAT_RUNNING   0x10	 // A worker is handling the job
#define TW_TASK_STAT_FINAL	   0x20	 // Finalizing the task
#define TW_TASK_STAT_COMPLETED 0x40	 // The job is completed
// The dependency can never be satisfied (eg. parent job failed). The job is removed from the flow
// graph
#define TW_TASK_STAT_ABORTED 0x80
#define TW_TASK_STAT_FAILED	 0x100
#define TW_TASK_STAT_TRANS	 0x200	// Status in transition, unused

/* Task priority
 * We do not use enum because it is not supported by openpa
 */
#define TW_TASK_PRIORITY_URGENT	  1
#define TW_TASK_PRIORITY_STANDARD 2
#define TW_TASK_PRIORITY_LAZY	  3

/* Predefined dependency handler
 * We do not use enum because it is not supported by openpa
 */
#define TW_TASK_DEP_NULL		 TW_Task_dep_null
#define TW_TASK_DEP_ALL_COMPLETE TW_Task_dep_all_complete

/* Task tag */
#define TW_TASK_TAG_ANY -1

/* Callback functions */

/**
 * @brief  Callback function for the task
 * @note   The engine execute the task by executing the task handler
 * @data Argument to the task handler
 * @retval 0 for success, error code otherwise
 */
typedef int (*TW_Task_handler_t) (void *data);

typedef struct TW_Task_dep_handler_t {
	int Mask;
	void *Data;
	/**
	 * @brief  Callback function to initialize or finalize the dependency state for the dependency
	 * callback function
	 * @note Initialize will be called when the task is commited to the engine. Finalize will be
	 * called when the task is removed from the engine.
	 * @task The task in question
	 * @num_deps Number of dependencies
	 * @data Internal status of the dependency handler
	 * @init 1 for initialize, 0 for finalize
	 * @retval 0 for success, error code otherwise
	 */
	int (*Init) (TW_Task_handle_t task, int num_deps, void **data);
	/**
	 * @brief  Callback function to initialize or finalize the dependency state for the dependency
	 * callback function
	 * @note Initialize will be called when the task is commited to the engine. Finalize will be
	 * called when the task is removed from the engine.
	 * @task The task in question
	 * @num_deps Number of dependencies
	 * @data Internal status of the dependency handler
	 * @init 1 for initialize, 0 for finalize
	 * @retval 0 for success, error code otherwise
	 */
	int (*Finalize) (TW_Task_handle_t task, void *data);
	/**
	 * @brief Callback function when the status of parent tasks changes
	 * @note The function will be called exactly once when the status of a paraent changes
	 * @task The task in question
	 * @parent The parent task whose status changed
	 * @new_status The status of the parent
	 * @data Internal status of the dependency handler
	 * @retval The status of the task
	 */
	int (*Status_change) (
		TW_Task_handle_t task, TW_Task_handle_t parent, int old_status, int new_status, void *data);
} TW_Task_dep_handler_t;

typedef void (*TW_Task_status_handler_t) (TW_Task_handle_t task, int status);

// Create, free
extern terr_t TW_Task_create (TW_Task_handler_t task_cb,
							  void *task_data,
							  TW_Task_dep_handler_t dep_handler,
							  int tag,
							  TW_Task_handle_t *task);	// Create a new task

extern terr_t TW_Task_create_ex (TW_Task_handler_t task_cb,
								 void *task_data,
								 TW_Task_dep_handler_t dep_handler,
								 int tag,
								 int priority,
								 TW_Task_status_handler_t stat_handler,
								 int status_mask,
								 TW_Task_handle_t *task);  // Create a new task

extern terr_t TW_Task_free (TW_Task_handle_t task);	 // Free up a task

extern terr_t TW_Task_create_barrier (TW_Engine_handle_t engine,  // Must have option of global
									  int dep_tag,
									  int tag,
									  TW_Task_handle_t *task);	// Create a new barrier task

// Controls
extern terr_t TW_Task_commit (TW_Task_handle_t task,
							  TW_Engine_handle_t engine);  // Put the task into the dag
extern terr_t TW_Task_commit_barrier (TW_Task_handle_t task);
extern terr_t TW_Task_retract (TW_Task_handle_t task);	// Remove task form the dag

// Mark complete without running
// Mark as running

// Wait
extern terr_t TW_Task_wait (TW_Task_handle_t task,
							int64_t timeout);  // Wait for a single task to complete. The
											   // calling thread joins the worker on the
											   // job being waited and all its parents.
extern terr_t TW_Task_wait_multi (TW_Task_handle_t *tasks,
								  int num,
								  int64_t timeout);	 // Wait for a multiple task to complete.

// Task dependency API
extern terr_t TW_Task_add_dep (TW_Task_handle_t child, TW_Task_handle_t parent);
extern terr_t TW_Task_rm_dep (TW_Task_handle_t child, TW_Task_handle_t parent);

// Info
extern terr_t TW_Task_get_status (TW_Task_handle_t task, int *statusp);
extern terr_t TW_Task_get_data (TW_Task_handle_t task, void **datap);
extern terr_t TW_Task_get_tag (TW_Task_handle_t task, int *tagp);

// get dep. tasks
// get task of certain tag

// Misc
const char *TW_Task_status_str (int status);

extern TW_Task_dep_handler_t TW_Task_dep_null;
extern TW_Task_dep_handler_t TW_Task_dep_all_complete;