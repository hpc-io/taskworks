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

#define TW_Task_STAT_ALL                                                                          \
	(TW_Task_STAT_PENDING | TW_Task_STAT_WAITING | TW_Task_STAT_QUEUEING | TW_Task_STAT_RUNNING | \
	 TW_Task_STAT_COMPLETE | TW_Task_STAT_ABORT | TW_Task_STAT_FAILED)

// Set event arg
extern terr_t TW_Event_arg_set_file (TW_Event_args_handle_t harg, TW_Fd_t fd, int events) {
	terr_t err = TW_SUCCESS;
	TW_File_event_args_t args;

	if ((events | TW_EVENT_FILE_ALL) != TW_EVENT_FILE_ALL) ASSIGN_ERR (TW_ERR_INVAL);

	args.fd		= fd;
	args.events = events;

	harg->type		= TW_Event_type_file;
	harg->args.file = args;

err_out:;
	return err;
}
extern terr_t TW_Event_arg_set_socket (TW_Event_args_handle_t harg,
									   TW_Socket_t socket,
									   int events) {
	terr_t err = TW_SUCCESS;
	TW_Socket_event_args_t args;

	if ((events | TW_EVENT_SOCKET_ALL) != TW_EVENT_SOCKET_ALL) ASSIGN_ERR (TW_ERR_INVAL);

	args.socket = socket;
	args.events = events;

	harg->type		  = TW_Event_type_socket;
	harg->args.socket = args;

err_out:;
	return err;
}

extern terr_t TW_Event_arg_set_timer (TW_Event_args_handle_t harg,
									  int64_t micro_sec,
									  int repeat_count) {
	terr_t err = TW_SUCCESS;
	TW_Timer_event_args_t args;

	if (repeat_count < 0 && repeat_count != TW_INFINITE) ASSIGN_ERR (TW_ERR_INVAL);
	if (micro_sec < 0) ASSIGN_ERR (TW_ERR_INVAL);

	args.micro_sec	  = micro_sec;
	args.repeat_count = repeat_count;

	harg->type		 = TW_Event_type_timer;
	harg->args.timer = args;

err_out:;
	return err;
}

extern terr_t TW_Event_arg_set_task (TW_Event_args_handle_t harg,
									 TW_Task_handle_t task,
									 int status) {
	terr_t err = TW_SUCCESS;
	TW_Task_event_args_t args;

	CHK_HANDLE (task, TW_Obj_type_task);

	if ((status | TW_Task_STAT_ALL) != TW_Task_STAT_ALL) ASSIGN_ERR (TW_ERR_INVAL);

	args.task	= task;
	args.status = status;

	harg->type		= TW_Event_type_task;
	harg->args.task = args;

err_out:;
	return err;
}

// Create, free
terr_t TW_Event_create (TW_Event_handler_t evt_cb,
						void *evt_data,
						TW_Event_args_t arg,
						TW_Event_handle_t *event) {
	terr_t err = TW_SUCCESS;
	TW_Obj_handle_t ep;

	ep = (TW_Obj_handle_t)TWI_Malloc (sizeof (TW_Obj_t));
	CHECK_PTR (ep)
	ep->objtype	   = TW_Obj_type_event;
	ep->evt_driver = TWI_Active_evt_driver;

	err = ep->driver->Event_create (evt_cb, evt_data, arg, ep, &(ep->driver_obj));
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