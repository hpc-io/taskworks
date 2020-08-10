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

#include "twabt.h"

TW_Driver_t TWABT_Driver = {
	/* Init */
	TWABT_Init,		 // Initialize the driver
	TWABT_Finalize,	 // Finalize the driver and free all resource

	/* Engine callbacks */
	TWABT_Engine_create,		   // Initialize the task engine
								   // with num_worker workers
	TWABT_Engine_free,			   // Finalize the task engine
	TWABT_Engine_do_work,		   // Run a single task using the calling thread
	TWABT_Engine_set_num_workers,  // Increase the number of worker by num_worker

	/* Task callbacks */
	TWABT_Task_create,			// Create a new task
	TWABT_Task_free,			// Free up a task
	TWABT_Task_create_barrier,	// Create a new barrier task
	TWABT_Task_commit,			// Put the task into the dag
	TWABT_Task_retract,			// Remove task form the dag
	TWABT_Task_wait_single,		// Wait for a single task to complete. The
								// calling thread joins the worker on the
								// job being waited and all its parents.
	TWABT_Task_wait,			// Wait for a multiple task to complete.
	TWABT_Task_add_dep, TWABT_Task_rm_dep, TWABT_Task_inq,

	/* Event callbacks */
	TWABT_Event_create,	 // Create a new event
	TWABT_Event_free,
	TWABT_Event_commit,	  // Commit event, start watching
	TWABT_Event_retract,  // Stop watching
};

TWI_Disposer_handle_t TWABTI_Disposer = NULL;

TWI_Ts_vector_handle_t TWABTI_Engines = NULL;
TWI_Ts_vector_handle_t TWABTI_Tasks	  = NULL;
TWI_Ts_vector_handle_t TWABTI_Events  = NULL;

static int TWABT_Abt_need_finalize = 0;

/**
 * @brief  Iinitialize the argobots driver
 * @note
 * @param  *argc:
 * @param  ***argv:
 * @retval
 */
terr_t TWABT_Init (int TWI_UNUSED *argc, char TWI_UNUSED ***argv) {
	terr_t err = TW_SUCCESS;
	int abterr;

	TWABTI_Disposer = TWI_Disposer_create ();
	CHECK_PTR (TWABTI_Disposer);

	// Initialize argobot if not yet innitialized
	if (ABT_initialized () != ABT_SUCCESS) {
		abterr = ABT_init (0, NULL);
		CHECK_ABTERR
		TWABT_Abt_need_finalize = 1;
	}

	// Initialize object lists
	TWABTI_Engines = TWI_Ts_vector_create ();
	CHECK_PTR (TWABTI_Engines)
	TWABTI_Tasks = TWI_Ts_vector_create ();
	CHECK_PTR (TWABTI_Tasks)
	TWABTI_Events = TWI_Ts_vector_create ();
	CHECK_PTR (TWABTI_Events)

err_out:;
	return err;
}

/**
 * @brief  Finalize the argobots driver
 * @note
 * @retval
 */
terr_t TWABT_Finalize (void) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i, size;

	// Free unfreed objects
	/*
	TWI_Ts_vector_lock (TWABTI_Events);
	size = (int)TWI_Ts_vector_size (TWABTI_Events);
	for (i = 0; i < size; i++) {
		err = TWABTI_Event_free (TWABTI_Events->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWABTI_Events);
	*/

	TWI_Ts_vector_lock (TWABTI_Tasks);
	size = (int)TWI_Ts_vector_size (TWABTI_Tasks);
	for (i = 0; i < size; i++) {
		err = TWABTI_Task_free (TWABTI_Tasks->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWABTI_Tasks);

	TWI_Ts_vector_lock (TWABTI_Engines);
	size = (int)TWI_Ts_vector_size (TWABTI_Engines);
	for (i = 0; i < size; i++) {
		err = TWABTI_Engine_free (TWABTI_Engines->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWABTI_Engines);

	// Free object list
	TWI_Ts_vector_free (TWABTI_Events);
	TWI_Ts_vector_free (TWABTI_Tasks);
	TWI_Ts_vector_free (TWABTI_Engines);

	// Finalize argobots
	if (TWABT_Abt_need_finalize) {
		abterr = ABT_finalize ();
		CHECK_ABTERR
	}

	TWI_Disposer_free (TWABTI_Disposer);

err_out:;
	return err;
}