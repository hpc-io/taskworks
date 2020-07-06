/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* TaskWorks engine APIs */

#pragma once

#ifdef ENABLE_PARALLEL
#include <mpi.h>
#endif

#include "taskworks.h"

typedef int terr_t;

typedef struct TW_Engine_t *TW_Engine_handle_t;

typedef struct TW_Driver_t *TW_Driver_handle_t;

typedef struct TW_Event_driver_t *TW_Event_driver_handle_t;

#ifdef ENABLE_PARALLEL
extern terr_t TW_Engine_init_par (
	int num_worker,
	MPI_Comm Comm,
	MPI_Info Info,
	TW_Engine_Pthread_driver_id_t work_driver,
	TW_Engine_Pevent_driver_id_t evt_driver,
	TW_Engine_handle_t *enginep);  // Initialize the task engine with num_worker workers
#endif

extern terr_t TW_Engine_create (int num_worker,
								TW_Driver_handle_t work_driver,
								TW_Event_driver_handle_t evt_driver,
								TW_Engine_handle_t *hpool);	 // Initialize the task engine
															 // with num_worker workers
extern terr_t TW_Engine_free (TW_Engine_handle_t hpool);	 // Finalize the task engine

// Better name? Progress, Make progress
extern terr_t TW_Engine_progress (
	TW_Engine_handle_t hpool);	// Run a single task using the calling thread

// Draw the task schedular loop as figure figram

// Control by time, running until exhust the time given?
extern terr_t TW_Engine_progress_until (
	int time,
	TW_Engine_handle_t hpool);	// Run a single task using the calling thread

extern terr_t TW_Engine_add_worker (
	int num_worker,
	TW_Engine_handle_t hpool);	// Increase the number of worker by num_worker

extern terr_t TW_Engine_rm_worker (
	int num_worker,
	TW_Engine_handle_t hpool);	// Decrease the number of worker by num_worker
