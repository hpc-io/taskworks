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

void TWLIBEVTI_Evt_file_cb (evutil_socket_t socket, short flags, void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_t arg;

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

		OPA_store_int (&(ep->status), EVT_STATUS_PENDING);
	}
}

void TWLIBEVTI_Evt_socket_cb (evutil_socket_t socket, short flags, void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg					   = ep->args;
		arg.type			   = TW_Event_type_socket;
		arg.args.socket.events = 0;
		arg.args.socket.socket = socket;
		if (flags & EV_READ) arg.args.socket.events |= TW_EVENT_FILE_READY_FOR_READ;
		if (flags & EV_WRITE) arg.args.socket.events |= TW_EVENT_FILE_READY_FOR_WRITE;
		ep->handler (&arg, ep->data);

		OPA_store_int (&(ep->status), EVT_STATUS_PENDING);
	}
}

void TWLIBEVTI_Evt_timer_cb (evutil_socket_t TWI_UNUSED socket, short flags, void *data) {
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)data;
	TW_Event_args_t arg;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_COMMITED, EVT_STATUS_RUNNING) ==
		EVT_STATUS_COMMITED) {
		arg					 = ep->args;
		arg.type			 = TW_Event_type_timer;
		arg.args.file.events = 0;
		if (flags & EV_READ) arg.args.file.events |= TW_EVENT_FILE_READY_FOR_READ;
		if (flags & EV_WRITE) arg.args.file.events |= TW_EVENT_FILE_READY_FOR_WRITE;
		ep->handler (&arg, ep->data);

		OPA_store_int (&(ep->status), EVT_STATUS_PENDING);
	}
}
