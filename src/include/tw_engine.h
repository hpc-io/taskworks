/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* TaskWorks engine APIs */

#pragma once

#include <stdint.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include "taskworks.h"

typedef struct TW_Obj_t *TW_Engine_handle_t;

/*
#ifdef HAVE_MPI
extern terr_t TW_Engine_init_par (
	int num_worker,
	MPI_Comm Comm,
	MPI_Info Info,
	TW_Engine_Pthread_driver_id_t engine_driver,
	TW_Engine_Pevent_driver_id_t evt_driver,
	TW_Engine_handle_t *engine);  // Initialize the task engine with num_worker workers
#endif
*/

extern terr_t TW_Engine_create (int num_worker,
								TW_Engine_handle_t *engine);  // Initialize the task engine
															  // with num_worker workers
extern terr_t TW_Engine_free (TW_Engine_handle_t engine);	  // Finalize the task engine

// Better name? Progress, Make progress
extern terr_t TW_Engine_progress (
	TW_Engine_handle_t engine);	 // Run a single task using the calling thread

// Draw the task schedular loop as figure figram

// Control by time, running until exhust the time given?
extern terr_t TW_Engine_progress_until (
	TW_Engine_handle_t engine,
	int64_t microsec);	// Run a single task using the calling thread

extern terr_t TW_Engine_set_num_worker (
	TW_Engine_handle_t engine, int num_worker);	 // Increase the number of worker by num_worker
