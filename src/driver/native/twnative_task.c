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

terr_t TWNATIVE_Task_create (TW_Task_handler_t task_cb,
							 void *task_data,
							 TW_Task_dep_handler_t dep_handler,
							 int tag,
							 int priority,
							 TW_Task_status_handler_t stat_handler,
							 int status_mask,
							 void *dispatcher_obj,
							 TW_Handle_t *htask) {	// Create a new task
	terr_t err			= TW_SUCCESS;
	TWNATIVE_Task_t *tp = NULL;

	DEBUG_ENTER_FUNC (2);

	tp = (TWNATIVE_Task_t *)TWI_Malloc (sizeof (TWNATIVE_Task_t));
	CHECK_PTR (tp)
	tp->parents = NULL;
	tp->childs	= NULL;
	tp->parents = TWI_Ts_vector_create ();
	CHECK_PTR (tp->parents)
	tp->childs = TWI_Ts_vector_create ();
	CHECK_PTR (tp->childs)
	err = TWI_Rwlock_init (&(tp->lock));
	CHECK_ERR
	tp->handler		   = task_cb;
	tp->data		   = task_data;
	tp->dep_handler	   = dep_handler;
	tp->status_handler = stat_handler;
	tp->status_mask	   = status_mask;
	tp->tag			   = tag;
	tp->priority	   = priority;
	tp->ep			   = NULL;
	tp->dispatcher_obj = dispatcher_obj;
	tp->self		   = NULL;
	OPA_store_int (&(tp->status), TW_TASK_STAT_IDLE);

	// Add to opened task list
	TWI_Ts_vector_push_back (TWNATIVEI_Tasks, tp);

	*htask = tp;

	DEBUG_PRINTF (1, "Created task %p, handler: %p, data: %p, tag: %d, priority: %d\n", (void *)tp,
				  (void *)(long long)(tp->handler), (void *)(tp->data), tp->tag, tp->priority);

err_out:;
	if (err) {
		if (tp) {
			TWI_Ts_vector_free (tp->parents);
			TWI_Ts_vector_free (tp->childs);
			TWI_Free (tp);
		}
	}

	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_free (TW_Handle_t htask) {	 // Free up a task
	int err				= TW_SUCCESS;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)htask;

	DEBUG_ENTER_FUNC (2);

	TWI_Ts_vector_swap_erase (TWNATIVEI_Tasks, tp);

	err = TWNATIVE_Taski_free (tp);

	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
									 int dep_tag,
									 int tag,
									 void *dispatcher_obj,
									 TW_Handle_t *htask) {	// Create a new barrier task
	terr_t err = TW_SUCCESS;
	int i, size;
	TWNATIVE_Task_t *tp, *pp;

	DEBUG_ENTER_FUNC (2);

	// Create task
	err =
		TWNATIVE_Task_create (NULL, NULL, TW_TASK_DEP_ALL_COMPLETE, tag, TW_TASK_PRIORITY_STANDARD,
							  NULL, 0, dispatcher_obj, (TW_Handle_t *)(&tp));
	CHECK_ERR

	// Set up dependency
	TWI_Ts_vector_lock (TWNATIVEI_Tasks);
	size = (int)TWI_Ts_vector_size (TWNATIVEI_Tasks);
	for (i = 0; i < size; i++) {
		pp = TWNATIVEI_Tasks->data[i];
		if (pp == tp) continue;
		if (engine == NULL || engine == pp->ep) {
			if (dep_tag == TW_TASK_TAG_ANY || dep_tag == pp->tag) {
				err = TWNATIVE_Task_add_dep (tp, pp);
				CHECK_ERR
			}
		}
	}
	TWI_Ts_vector_unlock (TWNATIVEI_Tasks);

	*htask = tp;

	DEBUG_PRINTF (1, "Created barrier task %p\n", (void *)tp);
err_out:;
	if (err) {
		if (tp) { TWNATIVE_Task_free (tp); }
	}

	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_commit (TW_Handle_t htask, TW_Handle_t engine) {  // Put the task into the dag
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	int ndep;
	TWI_Bool_t success;
	int pstatus, dstatus, tstatus;
	TWNATIVE_Engine_t *ep = (TWNATIVE_Engine_t *)engine;
	TWNATIVE_Task_t *tp	  = (TWNATIVE_Task_t *)htask;
	TWNATIVE_Task_t *pp;
	TWNATIVE_Task_dep_t *dp;

	DEBUG_ENTER_FUNC (2);

	DEBUG_PRINTF (1, "Committing task %p\n", (void *)tp);

	err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_IDLE, TW_TASK_STAT_DEPHOLD, &success);
	CHECK_ERR
	if (success != TWI_TRUE) { RET_ERR (TW_ERR_STATUS) }

	// Make sure everyone sees the status change
	TWI_Rwlock_wlock (&(tp->lock));
	TWI_Rwlock_wunlock (&(tp->lock));

	// Make sure no one delete a dep
	TWI_Ts_vector_lock (tp->parents);

	ndep = (int)TWI_Ts_vector_size (tp->parents);

	// Call dependency init
	if (tp->dep_handler.Init) {
		abterr = tp->dep_handler.Init (tp->dispatcher_obj, ndep, &(tp->dep_handler.Data));
		if (abterr != 0) { RET_ERR (TW_ERR_DEP_INIT) }
	}

	// Set engine, this must be done before exposing the task to threads
	tp->ep = ep;

	if (ndep > 0) {
		// Wire up dep list on parents

		for (i = 0; i < ndep; i++) {
			dp = (TWNATIVE_Task_dep_t *)(tp->parents->data[i]);
			pp = (TWNATIVE_Task_t *)OPA_load_ptr (&(dp->parent));

			// Add to parent dep list
			if (pp) {
				err = TWI_Ts_vector_push_back (pp->childs, dp);
				CHECK_ERR
			}
		}

		tstatus = TW_TASK_STAT_DEPHOLD;
		for (i = 0; i < ndep; i++) {
			dp = (TWNATIVE_Task_dep_t *)(tp->parents->data[i]);
			pp = (TWNATIVE_Task_t *)OPA_load_ptr (&(dp->parent));

			if (pp) {
				pstatus = OPA_load_int (&(pp->status));
				dstatus = OPA_load_int (&(dp->status));
				if (OPA_cas_int (&(dp->status), dstatus, pstatus) == dstatus) {
					if (tp->dep_handler.Mask & pstatus) {
						DEBUG_PRINTF (1, "notify task %p, status of task %p is %s\n",
									  (void *)(OPA_load_ptr (&(dp->child))), (void *)(pp),
									  TW_Task_status_str (pstatus));
						tstatus =
							tp->dep_handler.Status_change (tp->dispatcher_obj, pp->dispatcher_obj,
														   dstatus, pstatus, tp->dep_handler.Data);
						if (tstatus != TW_TASK_STAT_DEPHOLD) break;
					}
				}
			}
		}
	} else {
		tstatus = TW_TASK_STAT_READY;
	}
	TWI_Ts_vector_unlock (tp->parents);

	err = TWNATIVE_Taski_update_status (tp, TW_TASK_STAT_DEPHOLD, tstatus, &success);
	CHECK_ERR

err_out:;

	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_retract (TW_Handle_t htask) {
	terr_t err = TW_SUCCESS;
	int status;
	TWI_Bool_t success;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)htask;

	DEBUG_ENTER_FUNC (2);

	while (1) {
		status = OPA_load_int (&(tp->status));
		if (status == TW_TASK_STAT_DEPHOLD || status == TW_TASK_STAT_READY) {
			err = TWNATIVE_Taski_update_status (tp, status, TW_TASK_STAT_ABORTED, &success);
			CHECK_ERR

			if (success == TWI_TRUE) {
				tp->ep = NULL;
				break;
			}
		} else {
			RET_ERR (TW_ERR_STATUS)
		}
	}

	DEBUG_PRINTF (1, "Task %p retracted\n", (void *)tp);

err_out:;
	DEBUG_EXIT_FUNC (2);
	return err;
}

// Wait for a single task to complete. The
// calling thread joins the worker on the
// job being waited and all its parents.
terr_t TWNATIVE_Task_wait_single (TW_Handle_t htask, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int stat;
	ttime_t stoptime;
	TWI_Bool_t have_task;
	TWNATIVE_Engine_t *ep;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)htask;

	DEBUG_ENTER_FUNC (2);

	DEBUG_PRINTF (1, "Waiting for task %p\n", (void *)tp);

	// stat = OPA_load_int (&(tp->status));

	ep	 = tp->ep;	// tp->ep will be set to NULL once a thread completes tp, make a copy
	stat = OPA_load_int (&(tp->status));
	if (stat < TW_TASK_STAT_RUNNING) {	// Hasn't been ran
										// Raise priority to highest
		// TWNATIVE_Taski_set_priority_r (tp, TW_TASK_PRIORITY_RESERVED);
		TWI_Disposer_join (TWNATIVEI_Disposer);
		if (timeout == TW_TIMEOUT_NEVER) {
			do {
				err = TWNATIVE_Engine_scheduler_core (ep, &have_task);
				CHECK_ERR
				// Check current status
			} while (OPA_load_int (&(tp->status)) < TW_TASK_STAT_RUNNING);
		} else {
			stoptime = TWI_Time_now () + timeout;
			do {
				err = TWNATIVE_Engine_scheduler_core (ep, &have_task);
				CHECK_ERR
			} while ((TWI_Time_now () < stoptime) &&
					 (OPA_load_int (&(tp->status)) < TW_TASK_STAT_RUNNING));
		}
		// Run scheduler with workers

		TWI_Disposer_leave (TWNATIVEI_Disposer);
	}

	// Wait till complete
	while (OPA_load_int (&(tp->status)) < TW_TASK_STAT_COMPLETED)
		;

err_out:;
	DEBUG_EXIT_FUNC (2);
	return err;
}

// Wait for a multiple task to complete.
terr_t TWNATIVE_Task_wait (TW_Handle_t *htasks, int num_tasks, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int i;
	ttime_t stoptime, now;

	DEBUG_ENTER_FUNC (2);

	if (timeout == TW_TIMEOUT_NEVER) {
		for (i = 0; i < num_tasks; i++) {
			err = TWNATIVE_Task_wait_single (htasks[i], timeout);
			CHECK_ERR
		}
	} else {
		now		 = TWI_Time_now ();
		stoptime = now + timeout;
		for (i = 0; i < num_tasks && now < stoptime; i++) {
			err = TWNATIVE_Task_wait_single (htasks[i], stoptime - now);
			CHECK_ERR
			now = TWI_Time_now ();
		}
		if (i < num_tasks) { RET_ERR (TW_ERR_TIMEOUT) }
	}
err_out:;
	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_add_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err			= TW_SUCCESS;
	TWNATIVE_Task_t *cp = (TWNATIVE_Task_t *)child;
	TWNATIVE_Task_t *pp = (TWNATIVE_Task_t *)parent;
	TWNATIVE_Task_dep_t *dp;

	DEBUG_ENTER_FUNC (2);

	// Prevent task from commiting
	TWI_Rwlock_rlock (&(cp->lock));

	if (OPA_load_int (&(cp->status)) != TW_TASK_STAT_IDLE) ASSIGN_ERR (TW_ERR_STATUS)

	dp = (TWNATIVE_Task_dep_t *)TWI_Malloc (sizeof (TWNATIVE_Task_dep_t));
	CHECK_PTR (dp)

	OPA_store_ptr (&(dp->parent), pp);
	OPA_store_ptr (&(dp->child), cp);
	OPA_store_int (&(dp->ref), 2);
	OPA_store_int (&(dp->status), 0);

	// Insert to dep list
	err = TWI_Ts_vector_push_back (cp->parents, dp);
	CHECK_ERR

	DEBUG_PRINTF (1, "Task %p depends on task %p\n", (void *)cp, (void *)pp);

err_out:;
	TWI_Rwlock_runlock (&(cp->lock));

	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err = TW_SUCCESS;
	int i;
	int ndep;
	int status;
	TWNATIVE_Task_t *cp = (TWNATIVE_Task_t *)child;
	TWNATIVE_Task_t *pp = (TWNATIVE_Task_t *)parent;
	TWNATIVE_Task_dep_t *dp;

	DEBUG_ENTER_FUNC (2);

	// Prevent task from commiting
	TWI_Rwlock_rlock (&(cp->lock));

	if (OPA_load_int (&(cp->status)) != TW_TASK_STAT_IDLE) ASSIGN_ERR (TW_ERR_STATUS)

	if (status != TW_TASK_STAT_IDLE) ASSIGN_ERR (TW_ERR_STATUS)

	// Remove all dependencies
	TWI_Ts_vector_lock (cp->parents);
	ndep = (int)TWI_Ts_vector_size (cp->parents);
	for (i = 0; i < ndep; i++) {
		dp = cp->parents->data[i];
		if (OPA_load_ptr (&(dp->parent)) == pp) {
			OPA_store_ptr (&(dp->child), NULL);
			break;
		}
	}
	TWI_Ts_vector_unlock (cp->parents);
	if (i == ndep) { ASSIGN_ERR (TW_ERR_INVAL) }

	DEBUG_PRINTF (1, "Task %p no longer depends on task %p\n", (void *)cp, (void *)pp);

err_out:;
	TWI_Rwlock_runlock (&(cp->lock));
	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_inq (TW_Handle_t htask, TW_Task_inq_type_t inqtype, void *ret) {
	terr_t err			= TW_SUCCESS;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)htask;

	DEBUG_ENTER_FUNC (2);

	switch (inqtype) {
		case TW_Task_inq_type_status:
			*((int *)ret) = OPA_load_int (&(tp->status));
			break;
		case TW_Task_inq_type_data:
			*((void **)ret) = tp->data;
			break;
		case TW_Task_inq_type_tag:
			*((int *)ret) = tp->tag;
			break;
		default:
			ASSIGN_ERR (TW_ERR_INVAL);
			break;
	}

err_out:;
	DEBUG_EXIT_FUNC (2);
	return err;
}

terr_t TWNATIVE_Task_set_priority (TW_Handle_t task, int priority) {
	terr_t err			= TW_SUCCESS;
	TWNATIVE_Task_t *tp = (TWNATIVE_Task_t *)task;

	DEBUG_ENTER_FUNC (2);

	if (tp->priority != priority) {
		tp->priority = priority;

		// Reinsert into corresponding queue
		if (OPA_load_int (&(tp->status)) == TW_TASK_STAT_QUEUE) {
			err = TWNATIVE_Taski_queue (tp);
			CHECK_ERR
		}
	}
	DEBUG_PRINTF (1, "Task %p priority changed to %d\n", (void *)tp, tp->priority);

err_out:;

	DEBUG_EXIT_FUNC (2);
	return err;
}