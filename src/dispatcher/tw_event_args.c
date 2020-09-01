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

	args.poll = poll;
	args.data = data;

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
	void *dp;

	err = TW_Event_poll_mpi_data_create (comm, src, tag, &dp);
	CHECK_ERR

	err = TW_Event_arg_set_poll (harg, TWI_Event_poll_mpi, dp);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Event_arg_get_mpi (TW_Event_args_handle_t harg, int *flag, MPI_Status *stat) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;
	TW_Event_poll_mpi_data *dp;

	if (argp->type != TW_Event_type_poll) RET_ERR (TW_ERR_INVAL)

	dp = (TW_Event_poll_mpi_data *)argp->args.poll.data;

	*flag = dp->flag;
	*stat = dp->status;

	return TW_SUCCESS;
}
terr_t TW_Event_arg_set_mpi_req (TW_Event_args_handle_t harg, MPI_Request req) {
	terr_t err = TW_SUCCESS;
	void *dp;

	err = TW_Event_poll_mpi_req_data_create (req, &dp);
	CHECK_ERR

	err = TW_Event_arg_set_poll (harg, TWI_Event_poll_mpi_req, dp);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TW_Event_arg_get_mpi_req (TW_Event_args_handle_t harg, int *flag, MPI_Status *stat) {
	TW_Event_args_imp_t *argp = (TW_Event_args_imp_t *)harg;
	TW_Event_poll_mpi_req_data *dp;

	if (argp->type != TW_Event_type_poll) RET_ERR (TW_ERR_INVAL)

	dp = (TW_Event_poll_mpi_req_data *)argp->args.poll.data;

	*flag = dp->flag;
	*stat = dp->status;

	return TW_SUCCESS;
}
#endif
