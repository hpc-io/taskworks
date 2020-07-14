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

void *task_cb (void *task) {
	int ret;
	TWABT_Task_t *tp = task;

	ret = tp->handler (tp->data);

	if (ret == 0) {
		TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_COMPLETE);
	} else {
		TWABTI_Task_update_status (tp, TW_Task_STAT_RUNNING, TW_Task_STAT_FAILED);
	}

	return NULL;
}

terr_t TWABTI_Task_update_status (TWABT_Task_t *tp, int old_stat, int stat) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int new_stat, old_stat;
	TWABT_Task_t *itr;

	// Old stat need to be different
	if (old_stat == stat) return TW_SUCCESS;

	if (old_stat == OPA_cas_int (&(tp->status), old_stat, stat)) {
		// Notify child tasks
		itr = TWI_List_head (&(tp->childs));
		while (itr) {
			TWABTI_Task_dep_notify (itr, tp, tp->status);
			old_stat = OPA_load_int (&(itr->status));
			new_stat = itr->dep_cb (itr, tp, stat, itr->dep_stat);
			if (new_stat != old_stat) {
				err = TWABTI_Task_update_status (itr, old_stat, new_stat);
				CHK_ERR
			}
			itr = TWI_List_next (itr);
		}

		// Time for execution
		if (stat == TW_Task_STAT_QUEUEING) {}
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_queue (TWABT_Task_t *tp) {
	terr_t err = TW_SUCCESS;
	int abterr;

	if (tp->abt_task != ABT_TASK_NULL) {
		abterr = ABT_task_free (&(tp->abt_task));
		CHECK_ABTERR
	}

	abterr = ABT_task_create (tp->ep->pool, task_cb, tp, &(tp->abt_task));
	CHECK_ABTERR

err_out:;
	return err;
}