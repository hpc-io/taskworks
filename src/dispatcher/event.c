/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* User level Event API */

#pragma once

#include <time.h>

#include "taskworks_internal.h"

typedef struct TW_Event_driver {
	terr_t (*init) (TWI_Handle_t *hevt);	 // Initialize the event driver
	terr_t (*finalize) (TWI_Handle_t hevt);	 // Finalize the event driver

	terr_t (*create_time) (TWI_Handle_t driver_hevt,
						   int start,
						   int intval,
						   TWI_Handle_t *evt_hevt);	 // Create a time event

	// TODO: File event
	// TODO: Socket event
	// TODO: Task event

	// Consider: GPU event - is there cross-platform lib?

	// Control poll freq for diff type of evt?

	terr_t (*pull_evt) (TWI_Handle_t hevt);	 // Run 1 event
	terr_t (*free) (TWI_Handle_t hevt);		 // Free an event
} TW_Event_driver;

typedef struct TW_Event_t {
	TW_Event_type_t type;  // Type of the event

	TW_Event_handler_t cb;	// Function to run when the event occur
	void *cb_data;			// Input parameters to the event function

	int tag;  // Optional tag to be used by the application, do we need it?

	TWI_Handle_t hevt;
} TW_Event_t;