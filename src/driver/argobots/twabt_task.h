/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver functions */

#pragma once

#include "twabt.h"

typedef struct TWABT_Task_t {
	TWI_Rwlock_t lock;
	TW_Task_handler_t handler;			// Task handler
	void *data;							// Task handler data
	TW_Task_dep_handler_t dep_handler;	// Dependency handler
	TW_Task_status_handler_t status_handler;
	int status_mask;
	int tag;  // Tag for the user
	// ABT_task abt_task;					// Argobot task handle
	struct TWABT_Task_t **abt_task_ctx;
	OPA_int_t status;			   // Status of the task
	int priority;				   // Priority, currently not used
	TWI_Nb_list_handle_t parents;  // Tasks it depends on
	TWI_Nb_list_handle_t childs;   // Tasks depend on it
	TWABT_Engine_t *ep;			   // Engine it is commited to
	void *dispatcher_obj;		   // Corresponding structure at dispatcher level
} TWABT_Task_t;

typedef struct TWABT_Task_dep_t {
	// TWABT_Task_t *parent;  // parent task
	// TWABT_Task_t *child;   // child task
	OPA_ptr_t parent;
	OPA_ptr_t child;
	OPA_int_t status;  // Last status of the parent known to the child
	OPA_int_t ref;	   // Reference count
} TWABT_Task_dep_t;

/* Task callbacks */
terr_t TWABT_Task_create (TW_Task_handler_t task_cb,
						  void *task_data,
						  TW_Task_dep_handler_t dep_handler,
						  int tag,
						  int priority,
						  TW_Task_status_handler_t stat_handler,
						  int status_mask,
						  void *dispatcher_obj,
						  TW_Handle_t *task);		   // Create a new task
terr_t TWABT_Task_free (TW_Handle_t task);			   // Free up a task
terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  void *dispatcher_obj,
								  TW_Handle_t *task);  // Create a new barrier task
terr_t TWABT_Task_commit (TW_Handle_t task,
						  TW_Handle_t engine);	// Put the task into the dag
terr_t TWABT_Task_retract (TW_Handle_t task);	// Remove task form the dag
terr_t TWABT_Task_wait_single (TW_Handle_t task,
							   ttime_t timeout);  // Wait for a single task to complete. The
												  // calling thread joins the worker on the
												  // job being waited and all its parents.
terr_t TWABT_Task_wait (TW_Handle_t *tasks,
						int num_tasks,
						ttime_t timeout);  // Wait for a multiple task to complete.
terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent);
terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent);
terr_t TWABT_Task_inq (TW_Handle_t task, TW_Task_inq_type_t inqtype, void *ret);

/* Internal functions */
terr_t TWABTI_Task_free (TWABT_Task_t *task);
void TWABTI_Task_free_core (void *tp);
void TWABTI_Task_abttask_cb (void *task);
terr_t TWABTI_Task_run (TWABT_Task_t *tp, TWI_Bool_t *success);
terr_t TWABTI_Task_run_dep (TWABT_Task_t *tp, TWI_Bool_t *successp);
terr_t TWABTI_Task_update_status (TWABT_Task_t *tp, int old_stat, int new_stat, int *success);
terr_t TWABTI_Task_queue (TWABT_Task_t *tp);
terr_t TWABTI_Task_notify_parent_status (TWABT_Task_t *tp, int old_stat, int new_stat);