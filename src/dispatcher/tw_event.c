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
 * @brief  Event related APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include "dispatcher.h"
/// \endcond

// Create, free
terr_t TW_Event_create (TW_Event_handler_t evt_cb,
						void *evt_data,
						TW_Event_args_t arg,
						TW_Event_handle_t *event) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t ep;
	TW_Event_args_imp_t *arg_int;

	ep = (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	CHECK_PTR (ep)
	ep->objtype = TW_Obj_type_event;
	ep->driver	= TWI_Active_driver;

	arg_int = (TW_Event_args_imp_t *)(&arg);
	err		= ep->driver->Event_create (evt_cb, evt_data, *arg_int, ep, &(ep->driver_obj));
	CHECK_ERR

	*event = ep;

err_out:;
	if (err) { TWI_Free (ep); }
	return err;
}

terr_t TW_Event_free (TW_Event_handle_t event) {
	terr_t err = TW_SUCCESS;

	err = event->driver->Event_free (event->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

// Control
// Commit event, start watching
terr_t TW_Event_commit (TW_Event_handle_t event, TW_Engine_handle_t engine) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (event, TW_Obj_type_event)
	CHK_HANDLE (engine, TW_Obj_type_engine)

	err = event->driver->Event_commit (event->driver_obj, engine->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}
// Stop watching
terr_t TW_Event_retract (TW_Event_handle_t event) {
	terr_t err = TW_SUCCESS;

	CHK_HANDLE (event, TW_Obj_type_event)

	err = event->driver->Event_retract (event->driver_obj);
	CHECK_ERR

err_out:;
	return err;
}

const char *TW_Event_status_str (int status) {
	if (status & TW_EVENT_STAT_IDLE) {
		return "idle";
	} else if (status & TW_EVENT_STAT_WATCHING) {
		return "watching";
	} else if (status & TW_EVENT_STAT_TRIGGER) {
		return "triggered";
		//} else if (status & TW_EVENT_STAT_QUEUE) {
		//	return "queuing";
	} else if (status & TW_EVENT_STAT_RUNNING) {
		return "running";
		//} else if (status & TW_EVENT_STAT_COMPLETED) {
		//	return "completed";
	} else if (status & TW_EVENT_STAT_FAILED) {
		return "failed";
	} else if (status & TW_EVENT_STAT_TRANS) {
		return "transition";
	}

	return "unknown";
}