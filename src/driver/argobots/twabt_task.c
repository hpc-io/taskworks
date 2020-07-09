/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver engine implementation */

#include "twabt.h"

terr_t TWABT_Task_create (TW_Task_handler_t task_cb,
						  void *task_data,
						  TW_Task_dep_handler_t dep_cb,
						  int tag,
						  TW_Handle_t *htask) {}  // Create a new task

terr_t TWABT_Task_free (TW_Handle_t htask) {}		   // Free up a task
terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  TW_Handle_t *htask) {}			 // Create a new barrier task
terr_t TWABT_Task_commit (TW_Handle_t htask, TW_Handle_t engine) {}	 // Put the task into the dag
terr_t TWABT_Task_retract (TW_Handle_t htask) {}					 // Remove task form the dag
terr_t TWABT_Task_wait_single (TW_Handle_t htask) {}  // Wait for a single task to complete. The
													  // calling thread joins the worker on the
													  // job being waited and all its parents.
terr_t TWABT_Task_wait (TW_Handle_t *htasks, int *num_tasks, ttime_t timeout) {
}  // Wait for a multiple task to complete.
terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent) {}
terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent) {}
terr_t TWABT_Task_inq (TW_Handle_t htask, TW_Task_inq_type_t inqtype, int *ret) {}