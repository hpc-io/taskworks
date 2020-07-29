/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Libevent driver implementation */

#include "twlibevt.h"

TW_Event_driver_t TWLIBEVT_Driver = {
	/* Init */
	TWLIBEVT_Init,		// Initialize the driver
	TWLIBEVT_Finalize,	// Finalize the driver and free all resource

	/* Loop callbacks */
	TWLIBEVT_Loop_create,	// Initialize an event loop
	TWLIBEVT_Loop_free,		// Finalize the event loop
	TWLIBEVT_Loop_do_work,	// Check for a single event

	/* Event callbacks */
	TWLIBEVT_Event_create,	// Create a new event
	TWLIBEVT_Event_free,	// Free up an event
	TWLIBEVT_Event_commit,	// Commit an event
	TWLIBEVT_Event_retract	// Remove event from the loop
};

TWI_Ts_vector_handle_t TWLIBEVTI_Loops	= NULL;
TWI_Ts_vector_handle_t TWLIBEVTI_Events = NULL;

// static int TWLIBEVT_Abt_need_finalize = 0;

/**
 * @brief  Iinitialize the argobots driver
 * @note
 * @param  *argc:
 * @param  ***argv:
 * @retval
 */
terr_t TWLIBEVT_Init (int TWI_UNUSED *argc, char TWI_UNUSED ***argv) {
	terr_t err = TW_SUCCESS;

	// Initialize object lists
	TWLIBEVTI_Loops = TWI_Ts_vector_create ();
	CHECK_PTR (TWLIBEVTI_Loops)
	TWLIBEVTI_Events = TWI_Ts_vector_create ();
	CHECK_PTR (TWLIBEVTI_Events)

err_out:;
	return err;
}

/**
 * @brief  Finalize the argobots driver
 * @note
 * @retval
 */
terr_t TWLIBEVT_Finalize (void) {
	terr_t err = TW_SUCCESS;
	int i, size;

	TWI_Ts_vector_lock (TWLIBEVTI_Events);
	size = (int)TWI_Ts_vector_size (TWLIBEVTI_Events);
	for (i = 0; i < size; i++) {
		err = TWLIBEVT_Event_free (TWLIBEVTI_Events->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWLIBEVTI_Events);

	TWI_Ts_vector_lock (TWLIBEVTI_Loops);
	size = (int)TWI_Ts_vector_size (TWLIBEVTI_Loops);
	for (i = 0; i < size; i++) {
		err = TWLIBEVT_Loop_free (TWLIBEVTI_Loops->data[i]);
		CHECK_ERR
	}
	TWI_Ts_vector_unlock (TWLIBEVTI_Loops);

	// Free object list
	TWI_Ts_vector_free (TWLIBEVTI_Events);
	TWI_Ts_vector_free (TWLIBEVTI_Loops);

err_out:;
	return err;
}