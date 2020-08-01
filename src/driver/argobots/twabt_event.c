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

#include "twabt.h"

// Create a new event
terr_t TWABT_Event_create (TW_Event_handler_t evt_cb,
						   void *evt_data,
						   TW_Event_args_t args,
						   void *dispatcher_obj,
						   TW_Handle_t *hevt) {
	terr_t err		  = TW_SUCCESS;
	TWABT_Event_t *ep = NULL;

	ep = (TWABT_Event_t *)TWI_Malloc (sizeof (TWABT_Event_t));
	CHECK_PTR (ep)
	ep->handler		   = evt_cb;
	ep->data		   = evt_data;
	ep->dispatcher_obj = dispatcher_obj;
	ep->eng			   = NULL;
	ep->abt_task	   = ABT_TASK_NULL;
	if (args.type == TW_Event_type_task) {
		ep->driver_obj	= args.args.task.task;
		ep->status_flag = args.args.task.status;
		ep->driver		= NULL;
	} else {
		ep->driver = TWI_Active_evt_driver;
		ep->driver->Event_create (TWABTI_Event_cb, ep, args, &(ep->driver_obj));
	}

	*hevt = ep;

err_out:;
	if (err) {
		if (ep) { TWI_Free (ep); }
	}
	return err;
}
terr_t TWABT_Event_free (TW_Handle_t hevt) {
	terr_t err = TW_SUCCESS;
	int abterr;
	TWABT_Event_t *ep = (TWABT_Event_t *)hevt;

	if (ep->abt_task != ABT_TASK_NULL) {
		abterr = ABT_task_free (&(ep->abt_task));
		CHECK_ABTERR
	}

	if (ep->driver) { ep->driver->Event_free (ep->driver_obj); }

err_out:;
	TWI_Free (ep);
	return err;
}

// Commit event, start watching
terr_t TWABT_Event_commit (TW_Handle_t hevt, TW_Handle_t engine) {
	terr_t err			 = TW_SUCCESS;
	TWABT_Event_t *ep	 = (TWABT_Event_t *)hevt;
	TWABT_Engine_t *engp = (TWABT_Engine_t *)engine;

	ep->eng = engp;
	if (ep->driver) {
		if (engp->evt_driver != ep->driver) ASSIGN_ERR (TW_ERR_INCOMPATIBLE_OBJECT)
		err = ep->driver->Event_commit (ep->driver_obj, engp->evt_loop);
		CHECK_ERR
	} else {  // Currently, the only event handled by the driver is task repated event
		TWABT_Task_t *tp = (TWABT_Task_t *)ep->driver_obj;
		TWABT_Task_monitor_t *mp =
			(TWABT_Task_monitor_t *)TWI_Malloc (sizeof (TWABT_Task_monitor_t));

		CHECK_PTR (mp)
		mp->evt = ep;
		OPA_store_int (&(mp->ref), 2);

		TWI_Nb_list_insert_front (tp->events, mp);

		ep->driver_obj = mp;
	}

err_out:;
	if (err) { ep->eng = NULL; }
	return err;
}

// Stop watching
terr_t TWABT_Event_retract (TW_Handle_t hevt) {
	terr_t err		  = TW_SUCCESS;
	TWABT_Event_t *ep = (TWABT_Event_t *)hevt;

	if (ep->driver) {
		err = ep->driver->Event_retract (ep->driver_obj);
		CHECK_ERR
	} else {
		int iszero;
		TWABT_Task_monitor_t *mp = (TWABT_Task_monitor_t *)ep->driver_obj;

		iszero = OPA_decr_and_test_int (&(mp->ref));
		if (iszero) { TWI_Free (mp); }
	}
	ep->eng = NULL;

err_out:;
	return err;
}