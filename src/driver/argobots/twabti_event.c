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
typedef struct TWABTI_Event_task_data_t {
	TWABT_Event_t *ep;
	TW_Event_args_t *arg;
} TWABTI_Event_task_data_t;

void TWABTI_Event_task_cb (void *data) {
	TWABTI_Event_task_data_t *dp = (TWABTI_Event_task_data_t *)data;
	TWABT_Event_t *ep			 = dp->ep;

	ep->handler (ep->dispatcher_obj, dp->arg, ep->data);
	TWI_Free (dp);

	ep->driver->Event_commit (ep->driver_obj, ep->eng->evt_loop);
}

terr_t TWABTI_Event_cb (TW_Event_args_t *arg, void *data) {
	terr_t err = TW_SUCCESS;
	int abterr;
	TWABT_Event_t *ep = (TWABT_Event_t *)data;

	if (ep->eng->ness && 0) {  // Submit tasklet if there are workers
		TWABTI_Event_task_data_t *dp =
			(TWABTI_Event_task_data_t *)TWI_Malloc (sizeof (TWABTI_Event_task_data_t));
		CHECK_PTR (dp);

		dp->arg = arg;
		dp->ep	= ep;

		if (ep->abt_task != ABT_TASK_NULL) {
			abterr = ABT_task_free (&(ep->abt_task));
			CHECK_ABTERR
		}

		abterr = ABT_task_create (ep->eng->pools[0], TWABTI_Event_task_cb, dp, &(ep->abt_task));
		CHECK_ABTERR
	} else {  // Run it directly if no workers
		ep->handler (ep->dispatcher_obj, arg, ep->data);
	}

err_out:;
	return err;
}