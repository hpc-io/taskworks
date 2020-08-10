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
	TWI_Nb_list_itr_t itr;

	TWI_Rwlock_wlock (&(tp->lock));

	// Remove all dependencies
	TWI_Nb_list_inc_ref (tp->childs);
	for (itr = TWI_Nb_list_begin (tp->childs); itr != TWI_Nb_list_end (tp->childs);
		 itr = TWI_Nb_list_next (itr)) {  // Childs
		OPA_store_ptr (&(((TWABT_Task_dep_t *)itr->data)->parent), NULL);
	}
	TWI_Nb_list_dec_ref (tp->childs);
	TWI_Nb_list_inc_ref (tp->parents);
	for (itr = TWI_Nb_list_begin (tp->parents); itr != TWI_Nb_list_end (tp->parents);
		 itr = TWI_Nb_list_next (itr)) {  // Parents
		OPA_store_ptr (&(((TWABT_Task_dep_t *)itr->data)->child), NULL);
	}
	TWI_Nb_list_dec_ref (tp->parents);

	TWI_Rwlock_wunlock (&(tp->lock));

	return TWI_Disposer_dispose (TWABTI_Disposer, (void *)tp, TWABTI_Task_free_core);
}

void TWABTI_Task_free_core (void *obj) {  // Free up a task
	int is_zero;
	TWABT_Task_dep_t *dp;
	TWI_Nb_list_itr_t itr;
	TWABT_Task_t *tp = (TWABT_Task_t *)obj;

	TWI_Rwlock_wlock (&(tp->lock));

	DEBUG_PRINTF (1, "Freeing task %p\n", (void *)tp);

	// Remove all dependencies
	TWI_Nb_list_inc_ref (tp->childs);
	for (itr = TWI_Nb_list_begin (tp->childs); itr != TWI_Nb_list_end (tp->childs);
		 itr = TWI_Nb_list_next (itr)) {  // Childs
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) {
			// printf ("Freeing dep (%x, %x)\n", dp->child, dp->parent);
			TWI_Free (dp);
		}
	}
	TWI_Nb_list_dec_ref (tp->childs);
	TWI_Nb_list_inc_ref (tp->parents);
	for (itr = TWI_Nb_list_begin (tp->parents); itr != TWI_Nb_list_end (tp->parents);
		 itr = TWI_Nb_list_next (itr)) {  // Parents
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) {
			// printf ("Freeing dep (%x, %x)\n", dp->child, dp->parent);
			TWI_Free (dp);
		}
	}
	TWI_Nb_list_dec_ref (tp->parents);

	TWI_Rwlock_wunlock (&(tp->lock));

	TWI_Nb_list_free (tp->parents);
	TWI_Nb_list_free (tp->childs);
	TWI_Rwlock_finalize (&(tp->lock));
	TWI_Free (tp);
}

void TWABTI_Task_abttask_cb (void *task) {
	TWABT_Task_t *tp = *((TWABT_Task_t **)task);
	if (tp) {
		*(tp->abt_task_ctx) = NULL;
		TWABTI_Task_run (tp, NULL);
	}

	TWI_Disposer_dispose (TWABTI_Disposer, task, TWI_Free);
}

terr_t TWABTI_Task_run (TWABT_Task_t *tp, TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int ret;
	TWI_Bool_t success;

	// The task can already be done by the main thread or other ES from queue of other priority
	err = TWABTI_Task_update_status (tp, TW_TASK_STAT_QUEUE, TW_TASK_STAT_RUNNING, &success);
	CHECK_ERR

	if (success == TWI_TRUE) {
		// Wait for tp->abt_task_ctx be allocated

		// Wait until argobot task create
		// if (tp->ep->ness > 0) {
		//	while (tp->abt_task == NULL)
		//		;
		//}

		// Only run if there is callback function
		if (tp->handler) {
			ret = tp->handler (tp->data);
		} else
			ret = 0;

		if (ret == 0) {
			TWABTI_Task_update_status (tp, TW_TASK_STAT_RUNNING, TW_TASK_STAT_FINAL, NULL);
		} else {
			TWABTI_Task_update_status (tp, TW_TASK_STAT_RUNNING, TW_TASK_STAT_FAILED, NULL);
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
	TWI_Bool_t success = TWI_FALSE, notexist;
	int status;
	TWABT_Task_dep_t *dp;
	TWABT_Task_t *pp;
	TWI_Nb_list_itr_t i;

	DEBUG_PRINTF (1, "Trying to run task %p with calling thread\n", (void *)tp);

	while (success != TWI_TRUE) {
		status = OPA_load_int (&(tp->status));
		if (status == TW_TASK_STAT_QUEUE) {
			err = TWABTI_Task_run (tp, &success);
			CHECK_ERR
		} else if (status == TW_TASK_STAT_DEPHOLD) {
			TWI_Nb_list_inc_ref (tp->parents);
			for (i = TWI_Nb_list_begin (tp->parents); i != TWI_Nb_list_end (tp->parents);
				 i = TWI_Nb_list_next (i)) {
				dp = (TWABT_Task_dep_t *)i->data;
				pp = (TWABT_Task_t *)OPA_load_ptr (&(dp->parent));

				if (pp) {
					err = TWI_Hash_try_insert (h, pp, &notexist);
					CHECK_ERR
					if (notexist) {
						err = TWABTI_Task_run_dep_core (pp, h, &success);
						if (err != TW_SUCCESS) TWI_Nb_list_dec_ref (tp->parents);
						CHECK_ERR
					}
					if (success == TWI_TRUE) break;
				}
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

	if (success) *success = TWI_FALSE;

	// Old stat need to be different
	if (old_stat != new_stat) {
		if (old_stat == OPA_cas_int (&(tp->status), old_stat, new_stat)) {
			DEBUG_PRINTF (1, "task %p status changed to %s\n", (void *)tp,
						  TW_Task_status_str (new_stat));

			// Notify child tasks
			err = TWABTI_Task_notify_parent_status (tp, old_stat, new_stat);
			CHECK_ERR

			// Notify event
			if (tp->status_handler) {
				if (new_stat & tp->status_mask) {
					tp->status_handler (tp->dispatcher_obj, new_stat);
				}
			}

			// Take action based on new status
			switch (new_stat) {
				case TW_TASK_STAT_READY:
					if (tp->handler) {
						err = TWABTI_Task_queue (tp);
						CHECK_ERR
						err = TWABTI_Task_update_status (tp, TW_TASK_STAT_READY, TW_TASK_STAT_QUEUE,
														 NULL);
						CHECK_ERR
					} else {
						err = TWABTI_Task_update_status (tp, TW_TASK_STAT_READY, TW_TASK_STAT_FINAL,
														 NULL);
						CHECK_ERR
					}
					break;
				case TW_TASK_STAT_QUEUE:
					break;
				case TW_TASK_STAT_ABORTED:
					break;
				case TW_TASK_STAT_FAILED:
					break;
				case TW_TASK_STAT_FINAL:
					if (tp->ep) {
						err = TWI_Nb_list_del (tp->ep->tasks, tp);
						CHECK_ERR
						tp->ep = NULL;
					}
					if (tp->dep_handler.Finalize) {
						tp->dep_handler.Finalize (tp->dispatcher_obj, tp->dep_handler.Data);
					}
					err = TWABTI_Task_update_status (tp, TW_TASK_STAT_FINAL, TW_TASK_STAT_COMPLETED,
													 NULL);
					CHECK_ERR
					break;
				case TW_TASK_STAT_COMPLETED:
					break;
				default:
					break;
			}

			if (success) *success = TWI_TRUE;
		}
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_queue (TWABT_Task_t *tp) {
	terr_t err = TW_SUCCESS;
	int abterr;

	// if (tp->abt_task != ABT_TASK_NULL) {
	//	abterr = ABT_task_free (&(tp->abt_task));
	//	CHECK_ABTERR
	//}
	err = TWI_Nb_list_insert_front (tp->ep->tasks, tp);
	CHECK_ERR
	/* Argobot tasks can't be force removed, create them only when there are workers */
	if (tp->ep->ness > 0) {
		tp->abt_task_ctx	= (TWABT_Task_t **)TWI_Malloc (sizeof (TWABT_Task_t *));
		*(tp->abt_task_ctx) = tp;
		abterr				= ABT_task_create (tp->ep->pools[tp->priority], TWABTI_Task_abttask_cb,
								   tp->abt_task_ctx, NULL);
		CHECK_ABTERR
	} else {
		tp->abt_task_ctx = NULL;
	}

err_out:;
	return err;
}

terr_t TWABTI_Task_notify_parent_status (TWABT_Task_t *tp, int old_stat, int new_stat) {
	terr_t err = TW_SUCCESS;
	int stat_before, stat_after;
	TWI_Nb_list_itr_t itr;
	TWABT_Task_dep_t *dp;
	TWABT_Task_t *cp;

	// Notify child tasks
	TWI_Nb_list_inc_ref (tp->childs);
	itr = TWI_Nb_list_begin (tp->childs);
	while (itr != TWI_Nb_list_end (tp->childs)) {
		dp = itr->data;
		cp = (TWABT_Task_t *)OPA_load_ptr (&(dp->child));
		if (cp) {
			if (OPA_cas_int (&(dp->status), old_stat, new_stat) == old_stat) {
				stat_before = OPA_load_int (&(cp->status));
				if (stat_before == TW_TASK_STAT_DEPHOLD && (cp->dep_handler.Mask & new_stat)) {
					DEBUG_PRINTF (1, "notify task %p, status of task %p is %s\n", (void *)(cp),
								  (void *)tp, TW_Task_status_str (OPA_load_int (&(tp->status))));
					stat_after =
						cp->dep_handler.Status_change (cp->dispatcher_obj, tp->dispatcher_obj,
													   old_stat, new_stat, cp->dep_handler.Data);
					if (stat_before != stat_after) {
						err = TWABTI_Task_update_status (cp, stat_before, stat_after, NULL);
						CHECK_ERR
					}
				}
			}
		}
		itr = TWI_Nb_list_next (itr);
	}

err_out:;
	TWI_Nb_list_dec_ref (tp->childs);
	return err;
}