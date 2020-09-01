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

terr_t TWLIBEVT_Event_create (TW_Event_driver_handler_t evt_cb,
							  void *evt_data,
							  TW_Event_args_imp_t arg,
							  TW_Handle_t *event) {
	terr_t err = TW_SUCCESS;
	// int ret;
	TWLIBEVT_Event_t *ep;

	ep = (TWLIBEVT_Event_t *)TWI_Malloc (sizeof (TWLIBEVT_Event_t));
	CHECK_PTR (ep);
	ep->args	= arg;
	ep->handler = evt_cb;
	ep->data	= evt_data;
	ep->lp		= NULL;
	ep->event	= NULL;
	OPA_store_int (&(ep->status), EVT_STATUS_PENDING);
	OPA_store_ptr (&(ep->nlp), NULL);

	/*
	if ((arg.type == TW_Event_type_poll) && ep->args.args.poll.poll.Init) {
		ret = ep->args.args.poll.poll.Init (ep->args.args.poll.init_data, &ep->args.args.poll.data);
		if (ret) ASSIGN_ERR (err)
	}
	*/

	*event = ep;

err_out:;
	return err;
}

terr_t TWLIBEVT_Event_free (TW_Handle_t event) {
	terr_t err = TW_SUCCESS;
	// int ret;
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)event;

	if (ep->event) { event_free (ep->event); }

	/*
	if ((ep->args.type == TW_Event_type_poll) && ep->args.args.poll.poll.Finalize) {
		ret = ep->args.args.poll.poll.Finalize (ep->args.args.poll.data);
		if (ret) ASSIGN_ERR (TW_ERR_POLL_FIN)
	}
	*/

	TWI_Free (ep);

	// err_out:;
	return err;
}

terr_t TWLIBEVT_Event_commit (TW_Handle_t event, TW_Handle_t loop) {
	int err = TW_SUCCESS;
	int evterr;
	// int ret;
	short evt_flags = 0;
	evutil_socket_t fd;
	struct timeval tv, *tvp = NULL;
	event_callback_fn cb = NULL;
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)event;
	TWLIBEVT_Loop_t *lp	 = (TWLIBEVT_Loop_t *)loop;

	if (OPA_cas_int (&(ep->status), EVT_STATUS_PENDING, EVT_STATUS_COMMITED) !=
		EVT_STATUS_PENDING) {
		// ASSIGN_ERR (TW_ERR_STATUS)

		if (OPA_cas_ptr (&(ep->nlp), NULL, loop) == NULL) {	 // Put in pending slot
			if (OPA_load_int (&(ep->status)) ==
				EVT_STATUS_PENDING) {  // The other thread may not see the pending slot in time
									   // when status changes to pending
				if (OPA_cas_ptr (&(ep->nlp), loop, NULL) == loop) {
					if (OPA_cas_int (&(ep->status), EVT_STATUS_PENDING, EVT_STATUS_COMMITED) !=
						EVT_STATUS_PENDING) {  // Retry case, this time should success
						ASSIGN_ERR (TW_ERR_STATUS)
					}
				} else {  // If the other thread already took the pointer, we quit
					goto err_out;
				}
			} else {
				goto err_out;
			}
		} else {
			ASSIGN_ERR (TW_ERR_STATUS)
		}
	}

	evutil_timerclear (&tv);
	switch (ep->args.type) {
		case TW_Event_type_file:;
			if (ep->args.args.file.events & TW_EVENT_FILE_READY_FOR_READ) evt_flags |= EV_READ;
			if (ep->args.args.file.events & TW_EVENT_FILE_READY_FOR_WRITE) evt_flags |= EV_WRITE;
			fd = (evutil_socket_t)ep->args.args.file.fd;
			cb = TWLIBEVTI_Evt_file_cb;
			break;
		case TW_Event_type_socket:;
			if (ep->args.args.socket.events & TW_EVENT_SOCKET_READY_FOR_READ) evt_flags |= EV_READ;
			if (ep->args.args.socket.events & TW_EVENT_SOCKET_READY_FOR_WRITE)
				evt_flags |= EV_WRITE;
			fd = (evutil_socket_t)ep->args.args.socket.socket;
			cb = TWLIBEVTI_Evt_socket_cb;
			break;
		case TW_Event_type_timer:;
			tv.tv_usec = ep->args.args.timer.micro_sec;
			tvp		   = &tv;
			fd		   = -1;
			cb		   = TWLIBEVTI_Evt_timer_cb;
			break;
		case TW_Event_type_poll:;
			/*
			if (ep->args.args.poll.poll.Reset) {
				ret = ep->args.args.poll.poll.Reset (ep->args.args.poll.data);
				if (ret) ASSIGN_ERR (TW_ERR_POLL_RESET)
			}
			*/
			break;
		default:;
			ASSIGN_ERR (TW_ERR_INVAL)
	}

	if (ep->args.type == TW_Event_type_poll) {
		err = TWI_Ts_vector_push_back (lp->poll_events, ep);
		CHECK_ERR
	} else {
		ep->event = event_new (lp->base, fd, evt_flags, cb, ep);
		CHECK_LIBEVTPTR (ep->event)

		ep->lp = lp;

		evterr = event_add (ep->event, tvp);
		CHECK_LIBEVTERR
	}

err_out:;
	return err;
}
terr_t TWLIBEVT_Event_retract (TW_Handle_t hevt) {
	int err = TW_SUCCESS;
	int evterr;
	TWLIBEVT_Event_t *ep = (TWLIBEVT_Event_t *)hevt;

	evterr = event_del (ep->event);
	CHECK_LIBEVTERR

	event_free (ep->event);

	OPA_store_int (&(ep->status), EVT_STATUS_PENDING);

	ep->event = NULL;
	ep->lp	  = NULL;

err_out:;
	return err;
}
