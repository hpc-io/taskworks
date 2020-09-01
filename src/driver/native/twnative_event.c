/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver engine implementation */

#include "twnative.h"

// Create a new event
terr_t TWNATIVE_Event_create (TW_Event_handler_t evt_cb,
							  void *evt_data,
							  TW_Event_args_imp_t args,
							  void *dispatcher_obj,
							  TW_Handle_t *hevt) {
	terr_t err			 = TW_SUCCESS;
	TWNATIVE_Event_t *ep = NULL;

	ep = (TWNATIVE_Event_t *)TWI_Malloc (sizeof (TWNATIVE_Event_t));
	CHECK_PTR (ep)
	ep->handler		   = evt_cb;
	ep->data		   = evt_data;
	ep->dispatcher_obj = dispatcher_obj;
	ep->job			   = NULL;
	ep->eng			   = NULL;
	if (args.type == TW_Event_type_timer) {
		ep->repeat = args.args.timer.repeat_count;
	} else {
		ep->repeat = TW_INFINITE;
	}
	OPA_store_int (&(ep->status), TW_EVENT_STAT_IDLE);
	ep->driver = TWI_Active_evt_driver;
	ep->driver->Event_create (TWNATIVE_Eventi_cb, ep, args, &(ep->driver_obj));

	*hevt = ep;

	DEBUG_PRINTF (2, "Created event %p, handler: %p, data: %p\n", (void *)ep,
				  (void *)(long long)(ep->handler), (void *)(ep->data));

err_out:;
	if (err) {
		if (ep) { TWI_Free (ep); }
	}
	return err;
}
terr_t TWNATIVE_Event_free (TW_Handle_t hevt) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t success;
	TWNATIVE_Event_t *ep = (TWNATIVE_Event_t *)hevt;

	err = TWNATIVE_Eventi_update_status (ep, TW_EVENT_STAT_ANY ^ TW_EVENT_STAT_INVAL,
										 TW_EVENT_STAT_INVAL, &success);
	CHECK_ERR

	if (success) {
		err = ep->driver->Event_free (ep->driver_obj);
		CHECK_ERR

		err = TWI_Disposer_dispose (TWNATIVEI_Disposer, ep, TWNATIVE_Eventi_free);
		CHECK_ERR
	} else {
		ASSIGN_ERR (TW_ERR_STATUS)
	}

	DEBUG_PRINTF (2, "Freed event %p\n", (void *)ep);

err_out:;
	return err;
}

// Commit event, start watching
terr_t TWNATIVE_Event_commit (TW_Handle_t hevt, TW_Handle_t engine) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t success;
	TWNATIVE_Event_t *ep	= (TWNATIVE_Event_t *)hevt;
	TWNATIVE_Engine_t *engp = (TWNATIVE_Engine_t *)engine;

	err = TWNATIVE_Eventi_update_status (ep, TW_EVENT_STAT_IDLE | TW_EVENT_STAT_FAILED,
										 TW_EVENT_STAT_TRANS, &success);
	CHECK_ERR

	if (success) {
		ep->eng = engp;
		if (engp->evt_driver != ep->driver) ASSIGN_ERR (TW_ERR_INCOMPATIBLE_OBJECT)
		err = ep->driver->Event_commit (ep->driver_obj, engp->evt_loop);
		CHECK_ERR

		DEBUG_PRINTF (2, "Event %p commited to engine %p\n", (void *)ep, (void *)engp);

		err = TWNATIVE_Eventi_update_status (ep, TW_EVENT_STAT_TRANS, TW_EVENT_STAT_WATCHING,
											 &success);
		CHECK_ERR
	} else {
		ASSIGN_ERR (TW_ERR_STATUS)
	}

err_out:;
	if (err) {
		ep->eng = NULL;
		OPA_cas_int (&(ep->status), TW_EVENT_STAT_TRANS, TW_EVENT_STAT_IDLE);
	}
	return err;
}

// Stop watching
terr_t TWNATIVE_Event_retract (TW_Handle_t hevt) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t success;
	TWNATIVE_Event_t *ep = (TWNATIVE_Event_t *)hevt;

	if (OPA_load_int (&(ep->status)) == TW_EVENT_STAT_RUNNING ||
		OPA_load_int (&(ep->status)) == TW_EVENT_STAT_TRIGGER) {
		ep->repeat = 0;
	} else {
		err = TWNATIVE_Eventi_update_status (ep, TW_EVENT_STAT_WATCHING, TW_EVENT_STAT_TRANS,
											 &success);
		CHECK_ERR

		if (success) {
			err = ep->driver->Event_retract (ep->driver_obj);
			CHECK_ERR

			ep->eng = NULL;

			DEBUG_PRINTF (2, "Retracted event %p\n", (void *)ep);

			err = TWNATIVE_Eventi_update_status (ep, TW_EVENT_STAT_TRANS, TW_EVENT_STAT_IDLE,
												 &success);
			CHECK_ERR
		}
	}

err_out:;
	return err;
}