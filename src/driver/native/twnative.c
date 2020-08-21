/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver implementation */

#include "twnative.h"

TW_Driver_t TWNATIVE_Driver = {
	/* Init */
	TWNATIVE_Init,		// Initialize the driver
	TWNATIVE_Finalize,	// Finalize the driver and free all resource

	/* Engine callbacks */
	TWNATIVE_Engine_create,			  // Initialize the task engine
									  // with num_worker workers
	TWNATIVE_Engine_free,			  // Finalize the task engine
	TWNATIVE_Engine_do_work,		  // Run a single task using the calling thread
	TWNATIVE_Engine_set_num_workers,  // Increase the number of worker by num_worker

	/* Task callbacks */
	TWNATIVE_Task_create,		   // Create a new task
	TWNATIVE_Task_free,			   // Free up a task
	TWNATIVE_Task_create_barrier,  // Create a new barrier task
	TWNATIVE_Task_commit,		   // Put the task into the dag
	TWNATIVE_Task_retract,		   // Remove task form the dag
	TWNATIVE_Task_wait_single,	   // Wait for a single task to complete. The
								   // calling thread joins the worker on the
								   // job being waited and all its parents.
	TWNATIVE_Task_wait,			   // Wait for a multiple task to complete.
	TWNATIVE_Task_add_dep, TWNATIVE_Task_rm_dep, TWNATIVE_Task_inq,

	/* Event callbacks */
	TWNATIVE_Event_create,	// Create a new event
	TWNATIVE_Event_free,
	TWNATIVE_Event_commit,	 // Commit event, start watching
	TWNATIVE_Event_retract,	 // Stop watching
};

TWI_Disposer_handle_t TWNATIVEI_Disposer = NULL;

TWI_Ts_vector_handle_t TWNATIVEI_Engines = NULL;
TWI_Ts_vector_handle_t TWNATIVEI_Tasks	 = NULL;
TWI_Ts_vector_handle_t TWNATIVEI_Events	 = NULL;

TW_Thread_driver_handle_t TWNATIVEI_Driver = NULL;

/**
 * @brief  Iinitialize the argobots driver
 * @note
 * @param  *argc:
 * @param  ***argv:
 * @retval
 */
terr_t TWNATIVE_Init (int TWI_UNUSED *argc, char TWI_UNUSED ***argv) {
	terr_t err = TW_SUCCESS;
	TWNATIVEI_Thread_driver_t backend;
	char *env_val;

	// Select thread driver
	env_val = getenv ("TW_THREAD_BACKEND");
	if (env_val) {
		if (strcmp (env_val, "DEFAULT") == 0) {
			backend = TWNATIVEI_Thread_driver_default;
		}
#ifdef _WIN32
		else if (strcmp (env_val, "WIN32") == 0) {
			backend = TWNATIVEI_Thread_driver_win32;
		}
#else
		else if (strcmp (env_val, "POSIX") == 0) {
			backend = TWNATIVEI_Thread_driver_pthread;
		}
#endif
		else {
			ASSIGN_ERR (TW_ERR_INVAL_BACKEND)
		}
	} else {
		backend = TWNATIVEI_Thread_driver_default;
	}

	// Set backend driver
	switch (backend) {
		case TWNATIVEI_Thread_driver_default:  // Native backend for default
#ifdef _WIN32
		case TWNATIVEI_Thread_driver_win32:
			break;
#else
		case TWNATIVEI_Thread_driver_pthread:  // Argobots
			TWNATIVEI_Driver = &TWPOSIX_Driver;
			break;
#endif
		default:
			ASSIGN_ERR (TW_ERR_INVAL_BACKEND)
			break;
	}

	// Initialize the backend
	if (TWNATIVEI_Driver->init) { TWNATIVEI_Driver->init (argc, argv); }

	// Disposer
	TWNATIVEI_Disposer = TWI_Disposer_create ();
	CHECK_PTR (TWNATIVEI_Disposer);

	// Initialize object lists
	TWNATIVEI_Engines = TWI_Ts_vector_create ();
	CHECK_PTR (TWNATIVEI_Engines)
	TWNATIVEI_Tasks = TWI_Ts_vector_create ();
	CHECK_PTR (TWNATIVEI_Tasks)
	TWNATIVEI_Events = TWI_Ts_vector_create ();
	CHECK_PTR (TWNATIVEI_Events)

err_out:;
	return err;
}

/**
 * @brief  Finalize the argobots driver
 * @note
 * @retval
 */
terr_t TWNATIVE_Finalize (void) {
	terr_t err = TW_SUCCESS;
	int i, size;

	// Free all objects
	/*
	TWI_Ts_vector_lock (TWNATIVEI_Events);
	size = (int)TWI_Ts_vector_size (TWNATIVEI_Events);
	for (i = 0; i < size; i++) {
		err = TWNATIVE_Eventi_free (TWNATIVEI_Events->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWNATIVEI_Events);
	*/

	TWI_Ts_vector_lock (TWNATIVEI_Tasks);
	size = (int)TWI_Ts_vector_size (TWNATIVEI_Tasks);
	for (i = 0; i < size; i++) {
		err = TWNATIVE_Taski_free (TWNATIVEI_Tasks->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWNATIVEI_Tasks);

	TWI_Ts_vector_lock (TWNATIVEI_Engines);
	size = (int)TWI_Ts_vector_size (TWNATIVEI_Engines);
	for (i = 0; i < size; i++) {
		err = TWNATIVE_Enginei_free (TWNATIVEI_Engines->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWNATIVEI_Engines);

	// Free object list
	TWI_Ts_vector_free (TWNATIVEI_Events);
	TWI_Ts_vector_free (TWNATIVEI_Tasks);
	TWI_Ts_vector_free (TWNATIVEI_Engines);

	// Finalize the backend
	if (TWNATIVEI_Driver->finalize) { TWNATIVEI_Driver->finalize (); }

	TWI_Disposer_free (TWNATIVEI_Disposer);

err_out:;
	return err;
}