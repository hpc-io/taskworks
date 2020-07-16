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

void TWABTI_Task_abttask_cb (void *task) {
	terr_t err;
	int ret;
	int success;
	TWABT_Task_t *tp = task;

	err = TWABTI_Task_update_status (tp, TW_Task_STAT_QUEUEING, TW_Task_STAT_RUNNING, &success);
	CHECK_ERR

	if (success) {
		ret = tp->handler (tp->data);

		if (ret == 0) {
			TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_COMPLETE, NULL);
		} else {
			TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_FAILED, NULL);
		}
	}

err_out:;
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
					tp->ep = NULL;
					tp->dep_stat_cb (tp->dispatcher_obj, 0, &(tp->dep_stat), 0);
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

	abterr = ABT_task_create (tp->ep->pool, TWABTI_Task_abttask_cb, tp, &(tp->abt_task));
	CHECK_ABTERR

err_out:;
	return err;
}

terr_t TWABTI_Task_notify_parent_status (TWABT_Task_t *tp, int old_stat, int new_stat) {
	terr_t err = TW_SUCCESS;
	int stat_before, stat_after;
	TWI_List_itr_t itr;
	TWABT_Task_dep_t *dp;

	// Notify child tasks
	itr = TWI_List_begin (tp->childs);
	while (itr) {
		dp			= itr->data;
		stat_before = OPA_load_int (&(dp->child->status));
		if (stat_before == TW_Task_STAT_WAITING) {
			stat_after = dp->child->dep_cb (dp->child->dispatcher_obj, tp->dispatcher_obj, old_stat,
											new_stat, dp->child->dep_stat);
			if (stat_before != stat_after) {
				err = TWABTI_Task_update_status (dp->child, stat_before, stat_after, NULL);
				CHECK_ERR
			}
		}
		itr = TWI_List_next (itr);
	}

err_out:;
	return err;
}