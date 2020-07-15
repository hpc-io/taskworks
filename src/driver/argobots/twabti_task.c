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

void task_cb (void *task) {
	int ret;
	TWABT_Task_t *tp = task;

	ret = tp->handler (tp->data);

	if (ret == 0) {
		TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_COMPLETE);
	} else {
		TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_FAILED);
	}
}

terr_t TWABTI_Task_update_status (TWABT_Task_t *tp, int old_stat, int new_stat) {
	terr_t err = TW_SUCCESS;
	int abterr;

	// Old stat need to be different
	if (old_stat == new_stat) return TW_SUCCESS;

	if (old_stat == OPA_cas_int (&(tp->status), old_stat, new_stat)) {
		// Notify child tasks
		err = TWABTI_Task_notify_parent_status (tp, old_stat, new_stat);
		CHK_ERR

		// Time for execution
		if (new_stat == TW_Task_STAT_QUEUEING) {
			err = TWABTI_Task_queue (tp);
			CHK_ERR
		}
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_queue (TWABT_Task_t *tp) {
	terr_t err = TW_SUCCESS;
	int abterr;

	if (OPA_cas_int (&(tp->status), TW_Task_STAT_WAITING, TW_Task_STAT_RUNNING) !=
		TW_Task_STAT_WAITING) {
		RET_ERR (TW_ERR_STATUS)
	}

	if (tp->abt_task != ABT_TASK_NULL) {
		abterr = ABT_task_free (&(tp->abt_task));
		CHECK_ABTERR
	}

	abterr = ABT_task_create (tp->ep->pool, task_cb, tp, &(tp->abt_task));
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
	itr = TWI_List_head (tp->childs);
	while (itr) {
		dp			= itr->data;
		stat_before = OPA_load_int (&(dp->child->status));
		if (stat_before == TW_Task_STAT_WAITING) {
			stat_after = dp->child->dep_cb (dp->child->dispatcher_obj, tp->dispatcher_obj, old_stat,
											new_stat, dp->child->dep_stat);
			if (stat_before != stat_after) {
				err = TWABTI_Task_update_status (dp->child, stat_before, stat_after);
				CHK_ERR
			}
		}
		itr = TWI_List_next (itr);
	}

err_out:;
	return err;
}