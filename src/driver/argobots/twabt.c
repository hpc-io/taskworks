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

	// Initialize argobot if not yet innitialized
	if (ABT_initialized () != ABT_SUCCESS) {
		abterr = ABT_init (0, NULL);
		CHECK_ABTERR
		TWABT_Abt_need_finalize = 1;
	}

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

	if (TWABT_Abt_need_finalize) {
		abterr = ABT_finalize ();
		CHECK_ABTERR
	}

err_out:;
	return err;
}