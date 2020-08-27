/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Driver interface */

#pragma once

#include "taskworks_internal.h"
#include "twi_time.h"

typedef struct TW_Event_driver_t *TW_Event_driver_handle_t;

extern TW_Event_driver_handle_t TWI_Active_evt_driver;

typedef int (*TW_Event_driver_handler_t) (TW_Event_args_imp_t *arg, void *data);

typedef struct TW_Event_driver_t {
	/* Init */
	terr_t (*Init) (int *argc,
					char ***argv);	// Iinitialize the driver
	terr_t (*Finalize) (void);		// Finalize the driver

	/* Loop callbacks */
	terr_t (*Loop_create) (TW_Handle_t *loop);	// Initialize the task engine
												// with num_worker workers
	terr_t (*Loop_free) (TW_Handle_t engine);	// Finalize the task engine
	terr_t (*Loop_check_events) (TW_Handle_t engine,
								 ttime_t timeout);	// Check for a single event

	/* Event callbacks */
	terr_t (*Event_create) (TW_Event_driver_handler_t evt_cb,
							void *evt_data,
							TW_Event_args_imp_t arg,
							TW_Handle_t *event);  // Create a new event

	terr_t (*Event_free) (TW_Handle_t event);					   // Free up an event
	terr_t (*Event_commit) (TW_Handle_t event, TW_Handle_t loop);  // Commit an event
	terr_t (*Event_retract) (TW_Handle_t htask);				   // Remove event from the loop
} TW_Event_driver_t;

#ifdef HAVE_LIBEVENT
extern TW_Event_driver_t TWLIBEVT_Driver;
#endif