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

#include "twnative.h"

typedef struct TWNATIVE_Engine_t {
	TW_Handle_t lock;

	OPA_int_t cur_nt;	   // Current number of threads
	int nt;				   // Desired number of threads
	int nt_alloc;		   // Size of threads array
	TW_Handle_t *threads;  // Threads array

	TW_Handle_t njob;

	// TWI_Ts_vector_handle_t tasks;	// Commited tasks

	TWI_Nb_queue_handle_t queue[TWI_TASK_NUM_PRIORITY_LEVEL];  // Task pool of th engine

	TW_Thread_driver_handle_t driver;

	TW_Event_driver_handle_t evt_driver;
	void *evt_loop;
	TWI_Mutex_t evt_lock;

	void *dispatcher_obj;  // Corresponding structure at dispatcher level
} TWNATIVE_Engine_t;

typedef struct TWNATIVE_Thread_arg_t {
	TWNATIVE_Engine_t *ep;
	int id;
} TWNATIVE_Thread_arg_t;

typedef enum TWNATIVE_Job_type_t {
	TWNATIVE_Job_type_task,
	TWNATIVE_Job_type_event
} TWNATIVE_Job_type_t;

typedef struct TWNATIVE_Job_t {
	TWNATIVE_Job_type_t type;
	void *data;
} TWNATIVE_Job_t;

/* Engine callbacks */
terr_t TWNATIVE_Engine_create (int num_worker,
							   void *dispatcher_obj,
							   TW_Handle_t *engine);  // Initialize the task engine
													  // with num_worker workers
terr_t TWNATIVE_Engine_free (TW_Handle_t engine);	  // Finalize the task engine
terr_t TWNATIVE_Engine_do_work (TW_Handle_t engine,
								ttime_t timeout);  // Run a single task using the calling thread
terr_t TWNATIVE_Engine_set_num_workers (
	TW_Handle_t engine,
	int num_worker);  // Increase the number of worker by num_worker

/* Internal functions */
void *TWNATIVE_Engine_scheduler (void *data);
terr_t TWNATIVE_Engine_scheduler_core (TWNATIVE_Engine_t *ep, TWI_Bool_t *successp);
terr_t TWNATIVE_Enginei_free (TWNATIVE_Engine_t *ep);
void TWNATIVE_Enginei_free_core (void *obj);
terr_t TWNATIVE_Enginei_retract_task_r (TWNATIVE_Engine_t *ep, TW_Handle_t task);
