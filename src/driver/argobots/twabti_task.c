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

terr_t TWABTI_Task_free (TWABT_Task_t *tp) {  // Free up a task
	terr_t err = TW_SUCCESS;
	int abterr;
	int is_zero;
	TWABT_Task_dep_t *dp;
	TWI_Nb_list_itr_t itr;

	TWI_Rwlock_wlock (&(tp->lock));

	// Remove all dependencies
	TWI_Nb_list_inc_ref (tp->childs);
	itr = TWI_Nb_list_begin (tp->childs);  // Childs
	while (itr != TWI_Nb_list_end (tp->childs)) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) { TWI_Free (dp); }

		itr = TWI_Nb_list_next (itr);
	}
	TWI_Nb_list_dec_ref (tp->childs);
	TWI_Nb_list_inc_ref (tp->parents);
	itr = TWI_Nb_list_begin (tp->parents);	// Parents
	while (itr != TWI_Nb_list_end (tp->parents)) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) { TWI_Free (dp); }

		itr = TWI_Nb_list_next (itr);
	}
	TWI_Nb_list_dec_ref (tp->parents);

	TWI_Nb_list_inc_ref (tp->events);
	itr = TWI_Nb_list_begin (tp->events);  // Event listeners
	while (itr != TWI_Nb_list_end (tp->events)) {
		TWABT_Task_monitor_t *mp = (TWABT_Task_monitor_t *)itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(mp->ref));
		if (is_zero) { TWI_Free (mp); }

		itr = TWI_Nb_list_next (itr);
	}
	TWI_Nb_list_dec_ref (tp->events);

	if (tp->abt_task != ABT_TASK_NULL) {
		// Canceling cause seg fault in argobots
		// abterr = ABT_task_cancel (tp->abt_task);
		// CHECK_ABTERR
		abterr = ABT_task_free (&(tp->abt_task));
		CHECK_ABTERR
	}
	TWI_Rwlock_wunlock (&(tp->lock));

err_out:;
	TWI_Nb_list_free (tp->parents);
	TWI_Nb_list_free (tp->childs);
	TWI_Nb_list_free (tp->events);
	TWI_Rwlock_finalize (&(tp->lock));
	TWI_Free (tp);
	return err;
}

void TWABTI_Task_abttask_cb (void *task) { TWABTI_Task_run ((TWABT_Task_t *)task, NULL); }

terr_t TWABTI_Task_run (TWABT_Task_t *tp, TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int ret;
	TWI_Bool_t success;

	// The task can already be done by the main thread or other ES from queue of other priority
	err = TWABTI_Task_update_status (tp, TW_Task_STAT_QUEUEING, TW_Task_STAT_RUNNING, &success);
	CHECK_ERR

	if (success == TWI_TRUE) {
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

	while (success != TWI_TRUE) {
		status = OPA_load_int (&(tp->status));
		if (status == TW_Task_STAT_QUEUEING) {
			err = TWABTI_Task_run (tp, &success);
			CHECK_ERR
		} else if (status == TW_Task_STAT_WAITING) {
			TWI_Nb_list_dec_ref (tp->parents);
			i = TWI_Nb_list_begin (tp->parents);
			while (i != TWI_Nb_list_end (tp->parents)) {
				dp = (TWABT_Task_dep_t *)i->data;

				if (OPA_load_int (&(dp->ref)) == 2) {
					if (TWI_Hash_insert (h, dp->parent) == TW_SUCCESS) {
						err = TWABTI_Task_run_dep_core (dp->parent, h, &success);
						if (err != TW_SUCCESS) TWI_Nb_list_dec_ref (tp->parents);
						CHECK_ERR
					}
					if (success == TWI_TRUE) break;
				}

				i = TWI_Nb_list_next (i);
			}
			TWI_Nb_list_dec_ref (tp->parents);
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

terr_t TWABTI_Task_update_status (TWABT_Task_t *tp,
								  int old_stat,
								  int new_stat,
								  TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	TWI_Nb_list_itr_t i;
	TW_Event_args_t arg;

	if (success) *success = TWI_FALSE;

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

			if (success) *success = TWI_TRUE;

			arg.type			 = TW_Event_type_task;
			arg.args.task.task	 = tp->dispatcher_obj;
			arg.args.task.status = new_stat;
			TWI_Nb_list_inc_ref (tp->events);
			for (i = TWI_Nb_list_begin (tp->events); i != TWI_Nb_list_end (tp->events);
				 i = TWI_Nb_list_next (i)) {
				TWABT_Task_monitor_t *mp = (TWABT_Task_monitor_t *)i->data;

				if (OPA_load_int (&(mp->ref)) == 2) {
					if (mp->evt->status_flag & new_stat) {
						mp->evt->handler (mp->evt->dispatcher_obj, &arg, mp->evt->data);
					}
				}
			}
			TWI_Nb_list_dec_ref (tp->events);
		}
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_queue (TWABT_Task_t *tp) {
	terr_t err = TW_SUCCESS;
	int abterr;

	if (OPA_load_int (&(tp->status)) != TW_Task_STAT_QUEUEING) { RET_ERR (TW_ERR_STATUS) }

	if (tp->handler) {
		if (tp->abt_task != ABT_TASK_NULL) {
			abterr = ABT_task_free (&(tp->abt_task));
			CHECK_ABTERR
		}
		err = TWI_Nb_list_insert_front (tp->ep->tasks, tp);
		CHECK_ERR
		/* Argobot tasks can't be force removed, create them only when there are workers */
		if (tp->ep->ness > 0) {
			abterr = ABT_task_create (tp->ep->pools[tp->priority], TWABTI_Task_abttask_cb, tp,
									  &(tp->abt_task));
			CHECK_ABTERR
		}
	} else {
		err = TWABTI_Task_update_status (tp, TW_Task_STAT_QUEUEING, TW_Task_STAT_COMPLETE, NULL);
		CHECK_ERR
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
	TWI_Nb_list_inc_ref (tp->childs);
	itr = TWI_Nb_list_begin (tp->childs);
	while (itr != TWI_Nb_list_end (tp->childs)) {
		dp = itr->data;
		if (OPA_load_int (&(dp->ref)) == 2) {
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
		}
		itr = TWI_Nb_list_next (itr);
	}

err_out:;
	TWI_Nb_list_dec_ref (tp->childs);
	return err;
}