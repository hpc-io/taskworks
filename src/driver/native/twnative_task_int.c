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

terr_t TWNATIVE_Taski_free (TWNATIVE_Task_t *tp) {	// Free up a task
	TWI_Ts_vector_itr_t i;

	TWI_Rwlock_wlock (&(tp->lock));

	// Prevent it to be ran
	if (tp->job) { tp->job->data = NULL; }

	// Remove all dependencies
	TWI_Ts_vector_lock (tp->childs);
	for (i = TWI_Ts_vector_begin (tp->childs); i != TWI_Ts_vector_end (tp->childs);
		 i++) {	 // Childs
		OPA_store_ptr (&(((TWNATIVE_Task_dep_t *)(*i))->parent), NULL);
	}
	TWI_Ts_vector_unlock (tp->childs);

	TWI_Ts_vector_lock (tp->parents);
	for (i = TWI_Ts_vector_begin (tp->parents); i != TWI_Ts_vector_end (tp->parents);
		 i++) {	 // Childs
		OPA_store_ptr (&(((TWNATIVE_Task_dep_t *)(*i))->child), NULL);
	}
	TWI_Ts_vector_unlock (tp->parents);

	TWI_Rwlock_wunlock (&(tp->lock));

	return TWI_Disposer_dispose (TWNATIVEI_Disposer, (void *)tp, TWNATIVE_Taski_free_core);
}

void TWNATIVE_Taski_free_core (void *obj) {	 // Free up a task
	int is_zero;
	TWNATIVE_Task_dep_t *dp;
	TWI_Ts_vector_itr_t i;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)obj;

	TWI_Rwlock_wlock (&(tp->lock));

	DEBUG_PRINTF (2, "Freeing task %p\n", (void *)tp);

	// Remove all dependencies
	TWI_Ts_vector_lock (tp->childs);
	for (i = TWI_Ts_vector_begin (tp->childs); i != TWI_Ts_vector_end (tp->childs);
		 i++) {	 // Childs
		dp = *i;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) {
			// printf ("Freeing dep (%x, %x)\n", dp->child, dp->parent);
			TWI_Free (dp);
		}
	}
	TWI_Ts_vector_unlock (tp->childs);

	TWI_Ts_vector_lock (tp->parents);
	for (i = TWI_Ts_vector_begin (tp->parents); i != TWI_Ts_vector_end (tp->parents);
		 i++) {	 // Childs
		dp = *i;

		// Decrease ref count, if we are the last one, free the dep struct
		is_zero = OPA_decr_and_test_int (&(dp->ref));
		if (is_zero) {
			// printf ("Freeing dep (%x, %x)\n", dp->child, dp->parent);
			TWI_Free (dp);
		}
	}
	TWI_Ts_vector_unlock (tp->parents);

	TWI_Rwlock_wunlock (&(tp->lock));

	TWI_Ts_vector_free (tp->parents);
	TWI_Ts_vector_free (tp->childs);
	TWI_Rwlock_finalize (&(tp->lock));
	TWI_Free (tp);
}

terr_t TWNATIVE_Taski_run (TWNATIVE_Task_t *tp, TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int ret;
	TWI_Bool_t success;

	// The task can already be done by the main thread or other ES from queue of other priority
	err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_QUEUE, TW_TASK_STAT_RUNNING, &success);
	CHECK_ERR

	if (success == TWI_TRUE) {
		DEBUG_PRINTF (2, "Running task %p\n", (void *)tp);

		// Only run if there is callback function
		if (tp->handler) {
			ret = tp->handler (tp->data);
		} else
			ret = 0;

		if (ret == 0) {
			TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_RUNNING, TW_TASK_STAT_FINAL, NULL);
		} else {
			TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_RUNNING, TW_TASK_STAT_FAILED, NULL);
		}
	}

	if (successp) { *successp = success; }

err_out:;
	return err;
}

terr_t TWNATIVE_Taski_queue (TWNATIVE_Task_t *tp) {
	terr_t err = TW_SUCCESS;

	// Invalidate previous queue
	if (tp->job) { tp->job->data = NULL; }

	tp->job = (TWNATIVE_Job_t *)TWI_Malloc (sizeof (TWNATIVE_Job_t));
	CHECK_PTR (tp->job)
	tp->job->type = TWNATIVE_Job_type_task;
	tp->job->data = tp;
	err			  = TWI_Nb_queue_push (tp->ep->queue[tp->priority], tp->job);
	CHECK_ERR
	tp->ep->driver->sem.inc (tp->ep->njob);

err_out:;
	return err;
}

terr_t TWNATIVE_Taski_set_priority_r (TWNATIVE_Task_t *task, int priority) {
	terr_t err			= TW_SUCCESS;
	TWNATIVE_Task_t *tp = task;
	TWI_Ts_vector_itr_t i;
	TWI_Vector_handle_t s;
	TWNATIVE_Task_dep_t *dp;

	if ((priority > tp->priority)) {
		err = TWNATIVE_Task_set_priority (tp, priority);
		CHECK_ERR
	} else {
		s = TWI_Vector_create ();
		CHECK_PTR (s)

		err = TWI_Vector_push_back (s, tp);
		CHECK_ERR

		while (TWI_Vector_size (s) > 0) {
			TWI_Vector_pop_back (s, (void **)&(tp));
			// Raise raise the priority of parent tasks
			if (OPA_load_int (&(tp->status)) == TW_TASK_STAT_DEPHOLD) {
				for (i = TWI_Ts_vector_begin (tp->parents); i != TWI_Ts_vector_end (tp->parents);
					 i++) {
					dp = *i;
					if (OPA_load_int (
							&(((TWNATIVE_Task_t *)OPA_load_ptr (&(dp->parent)))->status)) &
						(TW_TASK_STAT_DEPHOLD | TW_TASK_STAT_QUEUE | TW_TASK_STAT_READY)) {
						err = TWI_Vector_push_back (s, OPA_load_ptr (&(dp->parent)));
						CHECK_ERR
					}
				}
			}
			err = TWNATIVE_Task_set_priority (tp, priority);
			CHECK_ERR
		}
	}

err_out:;
	return err;
}

terr_t TWNATIVE_Taski_update_status (TWNATIVE_Task_t *tp,
									 int old_stat,
									 int new_stat,
									 TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;

	if (success) *success = TWI_FALSE;

	// Old stat need to be different
	if (old_stat != new_stat) {
		if (old_stat == OPA_cas_int (&(tp->status), old_stat, new_stat)) {
			DEBUG_PRINTF (2, "task %p status changed to %s\n", (void *)tp,
						  TW_Task_status_str (new_stat));

			// Notify child tasks
			err = TWNATIVE_Taski_notify_parent_status (tp, old_stat, new_stat);
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
						err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_READY,
															TW_TASK_STAT_QUEUE, NULL);
						CHECK_ERR
						err = TWNATIVE_Taski_queue (tp);
						CHECK_ERR
					} else {
						err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_READY,
															TW_TASK_STAT_FINAL, NULL);
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
					if (tp->ep) { tp->ep = NULL; }
					if (tp->dep_handler.Finalize) {
						tp->dep_handler.Finalize (tp->dispatcher_obj, tp->dep_handler.Data);
					}
					err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_FINAL,
														TW_TASK_STAT_COMPLETED, NULL);
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

terr_t TWNATIVE_Taski_notify_parent_status (TWNATIVE_Task_t *tp, int old_stat, int new_stat) {
	terr_t err = TW_SUCCESS;
	int cur_stat;
	int stat_before, stat_after;
	TWI_Ts_vector_itr_t i;
	TWNATIVE_Task_dep_t *dp;
	TWNATIVE_Task_t *cp;

	// Notify child tasks
	TWI_Ts_vector_lock (tp->childs);
	for (i = TWI_Ts_vector_begin (tp->childs); i != TWI_Ts_vector_end (tp->childs); i++) {
		dp = *i;
		cp = (TWNATIVE_Task_t *)OPA_load_ptr (&(dp->child));
		if (cp) {
			cur_stat = OPA_load_int (&(dp->status));
			while (cur_stat < new_stat) {
				old_stat = cur_stat;
				cur_stat = OPA_cas_int (&(dp->status), old_stat, new_stat);
				if (cur_stat == old_stat) {
					stat_before = OPA_load_int (&(cp->status));
					if (stat_before == TW_TASK_STAT_DEPHOLD && (cp->dep_handler.Mask & new_stat)) {
						DEBUG_PRINTF (2, "notify task %p, status of task %p is %s\n", (void *)(cp),
									  (void *)tp,
									  TW_Task_status_str (OPA_load_int (&(tp->status))));
						stat_after = cp->dep_handler.Status_change (cp->dispatcher_obj,
																	tp->dispatcher_obj, old_stat,
																	new_stat, cp->dep_handler.Data);
						if (stat_before != stat_after) {
							err = TWNATIVE_Taski_update_status (cp, stat_before, stat_after, NULL);
							CHECK_ERR
						}
					} else {
						DEBUG_PRINTF (4, "child task %p of task %p not interested in status %s\n",
									  (void *)(cp), (void *)(OPA_load_ptr (&(dp->parent))),
									  TW_Task_status_str (OPA_load_int (&(dp->status))));
					}
					break;
				}
			}
#ifdef TWI_DEBUG
			if (cur_stat >= new_stat) {
				DEBUG_PRINTF (
					2,
					"notification of status %s to child task %p of task %p masked by status %s\n",
					TW_Task_status_str (new_stat), (void *)(cp),
					(void *)(OPA_load_ptr (&(dp->parent))),
					TW_Task_status_str (OPA_load_int (&(dp->status))));
			}
#endif
		}
	}

err_out:;
	TWI_Ts_vector_unlock (tp->childs);
	return err;
}