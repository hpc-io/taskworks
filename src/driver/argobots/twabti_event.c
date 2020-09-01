/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Eventworks. The full Eventworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver engine implementation */

#include "twabt.h"

void TWABTI_Event_task_cb (void *data) {
	TWABT_Event_t *ep = *((TWABT_Event_t **)data);

	TWI_Disposer_dispose (TWABTI_Disposer, data, TWI_Free);

	if (ep) {
		*(ep->abt_task_ctx) = NULL;
		TWABTI_Event_task_run (ep, NULL);
	}
}

void TWABTI_Event_task_run (TWABT_Event_t *ep, TWI_Bool_t *successp) {
	int ret;
	TWI_Bool_t success;

	TWABTI_Event_update_status (ep, TW_EVENT_STAT_TRIGGER, TW_EVENT_STAT_RUNNING, &success);
	if (success) {
		ret = ep->handler (ep->dispatcher_obj, (TW_Event_args_t *)(ep->arg), ep->data);
		if (ret == 0) {
			TWABTI_Event_update_status (ep, TW_EVENT_STAT_RUNNING, TW_EVENT_STAT_WATCHING,
										&success);

			if (ep->repeat > 0) ep->repeat--;
			if (ep->repeat != 0) { ep->driver->Event_commit (ep->driver_obj, ep->eng->evt_loop); }
		} else {
			TWABTI_Event_update_status (ep, TW_EVENT_STAT_RUNNING, TW_EVENT_STAT_FAILED, &success);
		}
	}

	if (successp) { *successp = success; }
}

terr_t TWABTI_Event_cb (TW_Event_args_imp_t *arg, void *data) {
	terr_t err = TW_SUCCESS;
	int abterr;
	TWI_Bool_t success;
	TWABT_Event_t *ep = (TWABT_Event_t *)data;

	TWABTI_Event_update_status (ep, TW_EVENT_STAT_WATCHING, TW_EVENT_STAT_TRANS, &success);

	if (success) {
		if (ep->eng->ness) {  // Submit tasklet if there are workers
			ep->abt_task_ctx = (TWABT_Event_t **)TWI_Malloc (sizeof (TWABT_Event_t *));
			CHECK_PTR (ep->abt_task_ctx);

			*(ep->abt_task_ctx) = ep;
			ep->arg				= arg;

			abterr =
				ABT_task_create (ep->eng->pools[0], TWABTI_Event_task_cb, ep->abt_task_ctx, NULL);
			CHECK_ABTERR

			TWABTI_Event_update_status (ep, TW_EVENT_STAT_TRANS, TW_EVENT_STAT_TRIGGER, NULL);
		} else {  // Run it directly if no workers
			TWABTI_Event_update_status (ep, TW_EVENT_STAT_TRANS, TW_EVENT_STAT_RUNNING, &success);
			if (success) {
				int ret;
				ret = ep->handler (ep->dispatcher_obj, (TW_Event_args_t *)arg, ep->data);
				if (ret == 0) {
					TWABTI_Event_update_status (ep, TW_EVENT_STAT_RUNNING, TW_EVENT_STAT_WATCHING,
												&success);

					if (ep->repeat > 0) ep->repeat--;
					if (ep->repeat != 0) {
						ep->driver->Event_commit (ep->driver_obj, ep->eng->evt_loop);
					}
				} else {
					TWABTI_Event_update_status (ep, TW_EVENT_STAT_RUNNING, TW_EVENT_STAT_FAILED,
												&success);
				}
			}
		}
	}

err_out:;
	return err;
}

void TWABTI_Event_free (void *hevt) {
	TWABT_Event_t *ep = (TWABT_Event_t *)hevt;

	TWI_Free (ep);
}

terr_t TWABTI_Event_update_status (TWABT_Event_t *ep,
								   int old_stat,
								   int new_stat,
								   TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int cur_stat, pre_stat;
	TWI_Bool_t success = TWI_TRUE;

	// Old stat need to be different
	if (old_stat != new_stat) {
		cur_stat = OPA_load_int (&(ep->status));
		do {
			pre_stat = cur_stat;

			if ((pre_stat | old_stat) != old_stat) {
				if (pre_stat == new_stat) {
					success = TWI_FALSE;  // Someone done it
					break;
				} else if (pre_stat == TW_EVENT_STAT_TRANS) {
					// In transition, wait until done
					while ((cur_stat = OPA_load_int (&(ep->status))) == TW_EVENT_STAT_TRANS)
						;
					continue;
				} else {
					success = TWI_FALSE;
					ASSIGN_ERR (TW_ERR_STATUS)	// Wrong status
				}
			}

			cur_stat = OPA_cas_int (&(ep->status), pre_stat, new_stat);
		} while (pre_stat != cur_stat);	 // Wait for our turn

		if (success) {
			DEBUG_PRINTF (2, "Event %p status changed to %s\n", (void *)ep,
						  TW_Event_status_str (new_stat));

			// Take action based on new status
			switch (new_stat) {
				case TW_EVENT_STAT_WATCHING:
					break;
				case TW_EVENT_STAT_TRIGGER:
					break;
				case TW_EVENT_STAT_FAILED:
					break;
				default:
					break;
			}
		}
	}

	if (successp) { *successp = success; }

err_out:;
	return err;
}