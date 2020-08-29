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

// Set event arg
terr_t TW_Event_arg_set_file (TW_Event_args_handle_t harg, TW_Fd_t fd, int events) {
	terr_t err = TW_SUCCESS;
	TW_File_event_args_t args;
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if ((events | TW_EVENT_FILE_ALL) != TW_EVENT_FILE_ALL) ASSIGN_ERR (TW_ERR_INVAL);

	args.fd		= fd;
	args.events = events;

	argp->type		= TW_Event_type_file;
	argp->args.file = args;

err_out:;
	return err;
}

terr_t TW_Event_arg_get_file (TW_Event_args_handle_t harg, TW_Fd_t *fd, int *events) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_file) RET_ERR (TW_ERR_INVAL)

	if (fd) { *fd = argp->args.file.fd; }
	if (events) { *events = argp->args.file.events; }

	return TW_SUCCESS;
}

terr_t TW_Event_arg_set_socket (TW_Event_args_handle_t harg, TW_Socket_t socket, int events) {
	terr_t err = TW_SUCCESS;
	TW_Socket_event_args_t args;
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if ((events | TW_EVENT_SOCKET_ALL) != TW_EVENT_SOCKET_ALL) ASSIGN_ERR (TW_ERR_INVAL);

	args.socket = socket;
	args.events = events;

	argp->type		  = TW_Event_type_socket;
	argp->args.socket = args;

err_out:;
	return err;
}
terr_t TW_Event_arg_get_socket (TW_Event_args_handle_t harg, TW_Socket_t *socket, int *events) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_socket) RET_ERR (TW_ERR_INVAL)

	if (socket) { *socket = argp->args.socket.socket; }
	if (events) { *events = argp->args.socket.events; }

	return TW_SUCCESS;
}

terr_t TW_Event_arg_set_timer (TW_Event_args_handle_t harg, int64_t micro_sec, int repeat_count) {
	terr_t err = TW_SUCCESS;
	TW_Timer_event_args_t args;
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (repeat_count < 0 && repeat_count != TW_INFINITE) ASSIGN_ERR (TW_ERR_INVAL);
	if (micro_sec < 0) ASSIGN_ERR (TW_ERR_INVAL);

	args.micro_sec	  = micro_sec;
	args.repeat_count = repeat_count;

	argp->type		 = TW_Event_type_timer;
	argp->args.timer = args;

err_out:;
	return err;
}
terr_t TW_Event_arg_get_timer (TW_Event_args_handle_t harg, int64_t *micro_sec, int *repeat_count) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_timer) RET_ERR (TW_ERR_INVAL)

	if (micro_sec) { *micro_sec = argp->args.timer.micro_sec; }
	if (repeat_count) { *repeat_count = argp->args.timer.repeat_count; }

	return TW_SUCCESS;
}

terr_t TW_Event_arg_set_poll (TW_Event_args_handle_t harg,
							  TW_Event_poll_handler_t poll,
							  void *data) {
	terr_t err = TW_SUCCESS;
	TW_Poll_event_args_t args;
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	args.poll	   = poll;
	args.init_data = data;

	argp->type		= TW_Event_type_poll;
	argp->args.poll = args;

	return err;
}
terr_t TW_Event_arg_get_poll (TW_Event_args_handle_t harg, void **data) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_poll) RET_ERR (TW_ERR_INVAL)

	if (data) { *data = argp->args.poll.data; }

	return TW_SUCCESS;
}

#ifdef HAVE_MPI
terr_t TW_Event_arg_set_mpi (TW_Event_args_handle_t harg, MPI_Comm comm, int src, int tag) {
	terr_t err = TW_SUCCESS;
	TW_Event_poll_mpi_data *dp;

	dp = (TW_Event_poll_mpi_data *)TWI_Malloc (sizeof (TW_Event_poll_mpi_data));
	CHECK_PTR (dp)

	dp->comm = comm;
	dp->src	 = src;
	dp->tag	 = tag;

	err = TW_Event_arg_set_poll (harg, TWI_Event_poll_mpi, dp);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Event_arg_get_mpi (TW_Event_args_handle_t harg, MPI_Comm *comm, int src, int tag) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_mpi) RET_ERR (TW_ERR_INVAL)

	*req = argp->args.mpi.req;

	return TW_SUCCESS;
}
terr_t TW_Event_arg_set_mpi_req (TW_Event_args_handle_t harg, MPI_Request req) {
	TW_Mpi_event_args_t args;
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	args.req = req;

	argp->type	   = TW_Event_type_mpi;
	argp->args.mpi = args;

	return TW_SUCCESS;
}
terr_t TW_Event_arg_get_mpi_req (TW_Event_args_handle_t harg, MPI_Request *req) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;

	if (argp->type != TW_Event_type_mpi) RET_ERR (TW_ERR_INVAL)

	*req = argp->args.mpi.req;

	return TW_SUCCESS;
}
#endif

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