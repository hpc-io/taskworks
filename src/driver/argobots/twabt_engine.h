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

typedef struct TWABT_Engine_t {
	int ness;									  // Number of threads (ES)
	int ness_alloc;								  // Size of schedulers and ess
	ABT_pool pools[TWI_TASK_NUM_PRIORITY_LEVEL];  // Task pool of th engine
	ABT_sched *schedulers;						  // Task scheduler
	ABT_xstream *ess;							  // Threads (ES)
	void *dispatcher_obj;						  // Corresponding structure at dispatcher level
	TWI_Nb_list_handle_t tasks;
	TW_Event_driver_handle_t evt_driver;
	void *evt_loop;
	TWI_Mutex_t evt_lock;
} TWABT_Engine_t;

/* Engine callbacks */
terr_t TWABT_Engine_create (int num_worker,
							void *dispatcher_obj,
							TW_Handle_t *engine);  // Initialize the task engine
												   // with num_worker workers
terr_t TWABT_Engine_free (TW_Handle_t engine);	   // Finalize the task engine
terr_t TWABT_Engine_do_work (TW_Handle_t engine,
							 ttime_t timeout);	// Run a single task using the calling thread
terr_t TWABT_Engine_set_num_workers (
	TW_Handle_t engine,
	int num_worker);  // Increase the number of worker by num_worker

/* Internal functions */
terr_t TWABTI_Engine_free (TWABT_Engine_t *engine);
void TWABTI_Engine_free_core (void *ep);