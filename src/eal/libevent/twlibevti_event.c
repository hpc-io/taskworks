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

static inline terr_t TWLIBEVTI_Evt_stat_reset (TWLIBEVT_Event_t *ep) {
	terr_t err = TW_SUCCESS;
	TWLIBEVT_Loop_t *lp;

	OPA_store_int (&(ep->status), EVT_STATUS_PENDING);

	// If need to recommit
	if ((lp = OPA_load_ptr (&(ep->nlp))) != NULL) {
		if (OPA_cas_ptr (&(ep->nlp), lp, NULL) == lp) {
			err = TWLIBEVT_Event_commit (ep, lp);
			CHECK_ERR
		}
	}
err_out:;
	return err;
}

void TWLIBEVTI_Evt_file_cb (evutil_socket_t socket, short flags, void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_imp_t arg;

	// Prevent the event from triggering again while the handler is being executed
	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg					 = ep->args;
		arg.type			 = TW_Event_type_file;
		arg.args.file.events = 0;
		arg.args.file.fd	 = socket;
		if (flags & EV_READ) arg.args.file.events |= TW_EVENT_FILE_READY_FOR_READ;
		if (flags & EV_WRITE) arg.args.file.events |= TW_EVENT_FILE_READY_FOR_WRITE;
		ep->handler (&arg, ep->data);

		TWLIBEVTI_Evt_stat_reset (ep);
	}
}

void TWLIBEVTI_Evt_socket_cb (evutil_socket_t socket, short flags, void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_imp_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg					   = ep->args;
		arg.type			   = TW_Event_type_socket;
		arg.args.socket.events = 0;
		arg.args.socket.socket = socket;
		if (flags & EV_READ) arg.args.socket.events |= TW_EVENT_FILE_READY_FOR_READ;
		if (flags & EV_WRITE) arg.args.socket.events |= TW_EVENT_FILE_READY_FOR_WRITE;
		ep->handler (&arg, ep->data);

		TWLIBEVTI_Evt_stat_reset (ep);
	}
}

void TWLIBEVTI_Evt_timer_cb (evutil_socket_t TWI_UNUSED socket,
							 short TWI_UNUSED flags,
							 void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_imp_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg		 = ep->args;
		arg.type = TW_Event_type_timer;
		ep->handler (&arg, ep->data);

		TWLIBEVTI_Evt_stat_reset (ep);
	}
}

void TWLIBEVTI_Evt_mpi_cb (TWLIBEVT_Event_t *ep, int flag, MPI_Status stat) {
	TW_Event_args_imp_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg				  = ep->args;
		arg.type		  = TW_Event_type_mpi;
		arg.args.mpi.flag = flag;
		arg.args.mpi.stat = stat;
		ep->handler (&arg, ep->data);

		TWLIBEVTI_Evt_stat_reset (ep);
	}
}

void TWLIBEVTI_Evt_poll_cb (TWLIBEVT_Event_t *ep) {
	TW_Event_args_imp_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg			  = ep->args;
		arg.type	  = TW_Event_type_poll;
		arg.args.poll = ep->args.args.poll;
		ep->handler (&arg, ep->data);

		TWLIBEVTI_Evt_stat_reset (ep);
	}
}
