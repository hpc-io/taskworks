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

void TWABTI_Task_abttask_cb (void *task) { TWABTI_Task_run ((TWABT_Task_t *)task, NULL); }

terr_t TWABTI_Task_run (TWABT_Task_t *tp, TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int ret;
	int success;

	// The task can already be done by the main thread or other ES from queue of other priority
	err = TWABTI_Task_update_status (tp, TW_Task_STAT_QUEUEING, TW_Task_STAT_RUNNING, &success);
	CHECK_ERR

	if (success) {
		// Only run if there is callback function
		if (tp->handler) {
			ret = tp->handler (tp->data);
		} else
			ret = 0;

		if (ret == 0) {
			TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_COMPLETE, NULL);
		} else {
			TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_FAILED, NULL);
		}
	}

	if (successp) { *successp = success; }

err_out:;
	return err;
}

static terr_t TWABTI_Task_run_dep_core (TWABT_Task_t *tp,
										TWI_Hash_handle_t h,
										TWI_Bool_t *successp) {
	terr_t err		   = TW_SUCCESS;
	TWI_Bool_t success = TWI_FALSE;
	int status;
	TWABT_Task_dep_t *dp;
	TWI_Nb_list_itr_t i;

	while (!success) {
		status = OPA_load_int (&(tp->status));
		if (status == TW_Task_STAT_QUEUEING) {
			err = TWABTI_Task_run (tp, &success);
			CHECK_ERR
		} else if (status == TW_Task_STAT_WAITING) {
			i = TWI_Nb_list_begin (tp->parents);
			while (i != TWI_Nb_list_end (tp->parents)) {
				dp = (TWABT_Task_dep_t *)i->data;

				if (OPA_load_int (&(dp->ref)) == 2) {
					if (TWI_Hash_insert (h, dp->parent) == TW_SUCCESS) {
						err = TWABTI_Task_run_dep_core (dp->parent, h, &success);
						CHECK_ERR
					}
					if (success) break;
				}

				i = TWI_Nb_list_next (i);
			}
		} else {
			break;
		}
	}

	if (successp) { *successp = success; }

err_out:;
	return err;
}

terr_t TWABTI_Task_run_dep (TWABT_Task_t *tp, TWI_Bool_t *successp) {
	terr_t err			= TW_SUCCESS;
	TWI_Hash_handle_t h = NULL;

	err = TWI_Hash_create (10, &h);
	CHECK_ERR

	err = TWI_Hash_insert (h, tp);
	CHECK_ERR

	err = TWABTI_Task_run_dep_core (tp, h, successp);
	CHECK_ERR

err_out:;
	if (h) TWI_Hash_free (h);
	return err;
}

terr_t TWABTI_Task_update_status (TWABT_Task_t *tp, int old_stat, int new_stat, int *success) {
	terr_t err = TW_SUCCESS;

	if (success) *success = 0;

	// Old stat need to be different
	if (old_stat != new_stat) {
		if (old_stat == OPA_cas_int (&(tp->status), old_stat, new_stat)) {
			// Notify child tasks
			err = TWABTI_Task_notify_parent_status (tp, old_stat, new_stat);
			CHECK_ERR

			// Take action based on new status
			switch (new_stat) {
				case TW_Task_STAT_QUEUEING:
					err = TWABTI_Task_queue (tp);
					CHECK_ERR
					break;
				case TW_Task_STAT_ABORT:
				case TW_Task_STAT_FAILED:
				case TW_Task_STAT_COMPLETE:
					if (tp->ep) {
						err = TWI_Nb_list_del (tp->ep->tasks, tp);
						CHECK_ERR
						tp->ep = NULL;
					}
					if (tp->dep_handler.Finalize) {
						tp->dep_handler.Finalize (tp->dispatcher_obj, tp->dep_handler.Data);
					}
					break;
				default:
					break;
			}

			if (success) *success = 1;
		}
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_queue (TWABT_Task_t *tp) {
	terr_t err = TW_SUCCESS;
	int abterr;

	if (OPA_load_int (&(tp->status)) != TW_Task_STAT_QUEUEING) { RET_ERR (TW_ERR_STATUS) }

	if (tp->abt_task != ABT_TASK_NULL) {
		abterr = ABT_task_free (&(tp->abt_task));
		CHECK_ABTERR
	}

	err = TWI_Nb_list_insert_front (tp->ep->tasks, tp);
	CHECK_ERR

	/* Argobot tasks can't be force removed, create them only when there are workers */
	if (tp->ep->ness > 0) {
		abterr = ABT_task_create (tp->ep->pool, TWABTI_Task_abttask_cb, tp, &(tp->abt_task));
		CHECK_ABTERR
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_notify_parent_status (TWABT_Task_t *tp, int old_stat, int new_stat) {
	terr_t err = TW_SUCCESS;
	int stat_before, stat_after;
	TWI_Nb_list_itr_t itr;
	TWABT_Task_dep_t *dp;

	// Notify child tasks
	itr = TWI_Nb_list_begin (tp->childs);
	while (itr != TWI_Nb_list_end (tp->childs)) {
		dp			= itr->data;
		stat_before = OPA_load_int (&(dp->child->status));
		if (stat_before == TW_Task_STAT_WAITING) {
			stat_after = dp->child->dep_handler.Status_change (
				dp->child->dispatcher_obj, tp->dispatcher_obj, old_stat, new_stat,
				dp->child->dep_handler.Data);
			if (stat_before != stat_after) {
				err = TWABTI_Task_update_status (dp->child, stat_before, stat_after, NULL);
				CHECK_ERR
			}
		}
		itr = TWI_Nb_list_next (itr);
	}

err_out:;
	return err;
}