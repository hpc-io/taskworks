/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file
 * @brief  User level common APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include <stdlib.h>
#include <string.h>

#include "dispatcher.h"
/// \endcond

TW_Event_driver_handle_t TWI_Active_evt_driver;
TW_Driver_handle_t TWI_Active_driver;

TW_Event_driver_handle_t evt_driver;
TW_Driver_handle_t driver;

/**
 * @brief  Initialize the TaskWorks library.
 * @note The library may remove TaskWorks related command line arguments.
 * @param  backend:
 * @param  event_backend:
 * @param  *argc: Command line argument count
 * @param  ***argv: Command line arguments
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Init (TW_Backend_t backend, TW_Event_backend_t event_backend, int *argc, char ***argv) {
	terr_t err = TW_SUCCESS;
	char *env_val;

#ifdef TWI_DEBUG
	env_val = getenv ("TW_DEBUG_MSG_LVL");
	if (env_val) {
		TWI_Debug_level = atoi (env_val);
	} else {
		TWI_Debug_level = 0;
	}
#endif

	// Overwrite backend option if environment variable is set
	env_val = getenv ("TW_BACKEND");
	if (env_val) {
		if (strcmp (env_val, "DEFAULT") == 0) {
			backend = TW_Backend_default;
		} else if (strcmp (env_val, "NATIVE") == 0) {
			backend = TW_Backend_native;
		}
#ifdef HAVE_ABT
		else if (strcmp (env_val, "ABT") == 0) {
			backend = TW_Backend_argobots;
		}
#endif
		else {
			ASSIGN_ERR (TW_ERR_INVAL_BACKEND)
		}
	}

	// Set backend driver
	switch (backend) {
		case TW_Backend_default:  // Native backend for default
		case TW_Backend_native:
			TWI_Active_driver = &TWNATIVE_Driver;
			break;
#ifdef HAVE_ABT
		case TW_Backend_argobots:  // Argobots
			TWI_Active_driver = &TWABT_Driver;
			break;
#endif
		default:
			ASSIGN_ERR (TW_ERR_INVAL_BACKEND)
			break;
	}

	// Overwrite event backend option if environment variable is set
	env_val = getenv ("TW_EVENT_BACKEND");
	if (env_val) {
		if (strcmp (env_val, "DEFAULT") == 0) {
			event_backend = TW_Event_backend_default;
		} else if (strcmp (env_val, "NONE") == 0) {
			event_backend = TW_Event_backend_none;
		}
#ifdef HAVE_LIBEVENT
		else if (strcmp (env_val, "LIBEVENT") == 0) {
			event_backend = TW_Event_backend_libevent;
		}
#endif
		else {
			ASSIGN_ERR (TW_ERR_INVAL_BACKEND)
		}
	}

	// Set event driver
	switch (event_backend) {
		case TW_Event_backend_default:	// The first available backend
#ifdef HAVE_LIBEVENT
		case TW_Event_backend_libevent:	 // Argobots
			TWI_Active_evt_driver = &TWLIBEVT_Driver;
			break;
#endif
		case TW_Event_backend_none:
			TWI_Active_evt_driver = NULL;
			break;
		default:
			ASSIGN_ERR (TW_ERR_INVAL_EVT_BACKEND)
			break;
	}

	// TODO: Init memory struct

	// TODO: Init obj list

	/* Assign predefined dependency handlers */
	TW_Task_dep_null		 = TWI_Task_dep_null;
	TW_Task_dep_all_complete = TWI_Task_dep_all_complete;

	/* Assign predefined event poll handlers */
	TW_Event_poll_mpi	  = TWI_Event_poll_mpi;
	TW_Event_poll_mpi_req = TWI_Event_poll_mpi_req;

	// Initialize the drivers
	err = TWI_Active_driver->Init (argc, argv);
	CHECK_ERR
	if (TWI_Active_evt_driver) {
		err = TWI_Active_evt_driver->Init (argc, argv);
		CHECK_ERR
	}

	// Initialize disposer for built in poll handler data
	TWI_Poll_disposer = TWI_Disposer_create ();
	CHECK_PTR (TWI_Poll_disposer)

err_out:;
	return err;
}

/**
 * @brief  Finalize the TaskWorks library.
 * @note   All resource will be freed. All TaskWorks objects will be closed.
 * @retval TW_SUCCESS on success or error code on failure
 */
terr_t TW_Finalize (void) {
	terr_t err = TW_SUCCESS;

	err = TWI_Active_driver->Finalize ();
	CHECK_ERR

	if (TWI_Active_evt_driver) {
		err = TWI_Active_evt_driver->Finalize ();
		CHECK_ERR
	}

	TWI_Disposer_free (TWI_Poll_disposer);

err_out:;
	return err;
}
