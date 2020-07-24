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

terr_t TWABT_Task_create (TW_Task_handler_t task_cb,
						  void *task_data,
						  TW_Task_dep_handler_t dep_handler,
						  int tag,
						  void *dispatcher_obj,
						  TW_Handle_t *htask) {	 // Create a new task
	terr_t err		 = TW_SUCCESS;
	TWABT_Task_t *tp = NULL;

	tp = (TWABT_Task_t *)TWI_Malloc (sizeof (TWABT_Task_t));
	CHECK_PTR (tp)
	err = TWI_Rwlock_init (&(tp->lock));
	CHECK_ERR
	tp->parents = NULL;
	tp->childs	= NULL;
	err			= TWI_Nb_list_create (&(tp->parents));
	CHECK_ERR
	err = TWI_Nb_list_create (&(tp->childs));
	CHECK_ERR
	tp->handler		   = task_cb;
	tp->data		   = task_data;
	tp->dep_handler	   = dep_handler;
	tp->tag			   = tag;
	tp->priority	   = 0;
	tp->ep			   = NULL;
	tp->abt_task	   = ABT_TASK_NULL;
	tp->dispatcher_obj = dispatcher_obj;
	OPA_store_int (&(tp->status), TW_Task_STAT_PENDING);

	// Add to opened task list
	TWI_Ts_vector_push_back (TWABTI_Tasks, tp);

	*htask = tp;

err_out:;
	if (err) {
		if (tp) {
			TWI_Nb_list_free (tp->parents);
			TWI_Nb_list_free (tp->childs);
			TWI_Free (tp);
		}
	}
	return err;
}

terr_t TWABT_Task_free (TW_Handle_t htask) {  // Free up a task
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;

	TWI_Ts_vector_swap_erase (TWABTI_Tasks, tp);

	return TWABTI_Task_free (tp);
}

terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  void *dispatcher_obj,
								  TW_Handle_t *htask) {	 // Create a new barrier task
	terr_t err = TW_SUCCESS;
	int i, size;
	TWABT_Task_t *tp, *pp;

	// Create task
	err = TWABT_Task_create (NULL, NULL, TW_TASK_DEP_ALL_COMPLETE, tag, dispatcher_obj,
							 (TW_Handle_t *)(&tp));
	CHECK_ERR

	// Set up dependency
	TWI_Ts_vector_lock (TWABTI_Tasks);
	size = (int)TWI_Ts_vector_size (TWABTI_Tasks);
	for (i = 0; i < size; i++) {
		pp = TWABTI_Tasks->data[i];
		if (pp == tp) continue;
		if (engine == NULL || engine == pp->ep) {
			if (dep_tag == TW_TASK_TAG_ANY || dep_tag == pp->tag) {
				err = TWABT_Task_add_dep (tp, pp);
				CHECK_ERR
			}
		}
	}
	TWI_Ts_vector_unlock (TWABTI_Tasks);

	*htask = tp;
err_out:;
	if (err) {
		if (tp) { TWABT_Task_free (tp); }
	}
	return err;
}

terr_t TWABT_Task_commit (TW_Handle_t htask, TW_Handle_t engine) {	// Put the task into the dag
	terr_t err = TW_SUCCESS;
	int abterr;
	int ndep;
	TWI_Bool_t success;
	int pstatus, dstatus, tstatus;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;
	TWABT_Task_t *tp   = (TWABT_Task_t *)htask;
	TWABT_Task_dep_t *dp;
	TWI_Nb_list_itr_t i;

	err = TWABTI_Task_update_status (tp, TW_Task_STAT_PENDING, TW_Task_STAT_WAITING, &success);
	CHECK_ERR
	if (success != TWI_TRUE) { RET_ERR (TW_ERR_STATUS) }

	// Prevent modification of dependencies
	TWI_Rwlock_wlock (&(tp->lock));

	// Count number of deps
	for (i = TWI_Nb_list_begin (tp->parents), ndep = 0; i != TWI_Nb_list_end (tp->parents);
		 i = TWI_Nb_list_next (i), ndep++)
		;

	// Call dependency init
	if (tp->dep_handler.Init) {
		abterr = tp->dep_handler.Init (tp->dispatcher_obj, ndep, &(tp->dep_handler.Data));
		if (abterr != 0) { RET_ERR (TW_ERR_DEP_INIT) }
	}

	// Wire up dep list on parents
	for (i = TWI_Nb_list_begin (tp->parents); i != TWI_Nb_list_end (tp->parents);
		 i = TWI_Nb_list_next (i)) {
		dp = i->data;

		// Add to parent dep list
		TWI_Nb_list_insert_front (dp->parent->childs, dp);
	}

	// Check if we need to notify dependency change
	i = TWI_Nb_list_begin (tp->parents);
	if (i != TWI_Nb_list_end (tp->parents)) {
		tstatus = TW_Task_STAT_WAITING;
		for (; i != TWI_Nb_list_end (tp->parents); i = TWI_Nb_list_next (i)) {
			dp = i->data;

			pstatus = OPA_load_int (&(dp->parent->status));
			dstatus = OPA_load_int (&(dp->status));
			if (OPA_cas_int (&(dp->status), dstatus, pstatus) == dstatus) {
				if (tp->dep_handler.Mask & pstatus) {
					tstatus = tp->dep_handler.Status_change (tp->dispatcher_obj,
															 dp->parent->dispatcher_obj, dstatus,
															 pstatus, tp->dep_handler.Data);
					if (tstatus != TW_Task_STAT_WAITING) break;
				}
			}
		}
	} else {
		tstatus = TW_Task_STAT_QUEUEING;
	}

	// Set engine
	tp->ep = ep;

	err = TWABTI_Task_update_status (tp, TW_Task_STAT_WAITING, tstatus, &success);
	CHECK_ERR

err_out:;
	TWI_Rwlock_wunlock (&(tp->lock));

	return err;
}

terr_t TWABT_Task_retract (TW_Handle_t htask) {
	terr_t err = TW_SUCCESS;
	int status;
	TWI_Bool_t success;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;

	while (1) {
		status = OPA_load_int (&(tp->status));
		if (status == TW_Task_STAT_WAITING || status == TW_Task_STAT_QUEUEING) {
			err = TWABTI_Task_update_status (tp, status, TW_Task_STAT_ABORT, &success);
			CHECK_ERR

			if (success == TWI_TRUE) break;
		} else {
			RET_ERR (TW_ERR_STATUS)
		}
	}

err_out:;
	return err;
}

// Wait for a single task to complete. The
// calling thread joins the worker on the
// job being waited and all its parents.
terr_t TWABT_Task_wait_single (TW_Handle_t htask, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int stat;
	ttime_t stoptime;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;

	if (timeout == TW_TIMEOUT_NEVER) {
		while (1) {
			stat = OPA_load_int (&(tp->status));
			if (stat == TW_Task_STAT_COMPLETE || stat == TW_Task_STAT_ABORT ||
				stat == TW_Task_STAT_FAILED)
				break;

			err = TWABTI_Task_run_dep (tp, NULL);
			CHECK_ERR
		}
	} else {
		stoptime = TWI_Time_now () + timeout;
		while (TWI_Time_now () < stoptime) {
			stat = OPA_load_int (&(tp->status));
			if (stat == TW_Task_STAT_COMPLETE || stat == TW_Task_STAT_ABORT ||
				stat == TW_Task_STAT_FAILED)
				break;
			if (tp->ep && tp->ep->ness == 0) {
				err = TWABTI_Task_run_dep (tp, NULL);
				CHECK_ERR
			} else {
			}
		}
		if (stat != TW_Task_STAT_COMPLETE) { ASSIGN_ERR (TW_ERR_TIMEOUT) }
	}
err_out:;
	return err;
}

// Wait for a multiple task to complete.
terr_t TWABT_Task_wait (TW_Handle_t *htasks, int num_tasks, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int i;
	ttime_t stoptime, now;

	if (timeout == TW_TIMEOUT_NEVER) {
		for (i = 0; i < num_tasks; i++) {
			err = TWABT_Task_wait_single (htasks[i], timeout);
			CHECK_ERR
		}
	} else {
		now		 = TWI_Time_now ();
		stoptime = now + timeout;
		for (i = 0; i < num_tasks && now < stoptime; i++) {
			err = TWABT_Task_wait_single (htasks[i], stoptime - now);
			CHECK_ERR
			now = TWI_Time_now ();
		}
		if (i < num_tasks) { RET_ERR (TW_ERR_TIMEOUT) }
	}
err_out:;
	return err;
}

terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err		 = TW_SUCCESS;
	TWABT_Task_t *cp = (TWABT_Task_t *)child;
	TWABT_Task_t *pp = (TWABT_Task_t *)parent;
	TWABT_Task_dep_t *dp;

	TWI_Rwlock_rlock (&(cp->lock));

	dp = (TWABT_Task_dep_t *)TWI_Malloc (sizeof (TWABT_Task_dep_t));
	CHECK_PTR (dp)

	dp->parent = pp;
	dp->child  = cp;
	OPA_store_int (&(dp->ref), 2);
	OPA_store_int (&(dp->status), 0);

	// Insert to dep list
	err = TWI_Nb_list_insert_front (cp->parents, dp);
	CHECK_ERR

err_out:;
	TWI_Rwlock_runlock (&(cp->lock));

	return err;
}

terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err = TW_SUCCESS;
	int is_zero;
	TWABT_Task_t *cp = (TWABT_Task_t *)child;
	TWABT_Task_t *pp = (TWABT_Task_t *)parent;
	TWABT_Task_dep_t *dp;
	TWI_Nb_list_itr_t itr;

	TWI_Rwlock_rlock (&(cp->lock));

	// Remove all dependencies
	itr = TWI_Nb_list_begin (cp->parents);	// Parents
	while (itr) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		if (dp->parent == pp) {
			is_zero = OPA_decr_and_test_int (&(dp->ref));
			if (is_zero) { TWI_Free (dp); }
			break;
		}

		itr = TWI_Nb_list_next (itr);
	}
	if (!itr) { ASSIGN_ERR (TW_ERR_NOT_FOUND) }

err_out:;
	TWI_Rwlock_runlock (&(cp->lock));

	return err;
}

terr_t TWABT_Task_inq (TW_Handle_t htask, TW_Task_inq_type_t inqtype, void *ret) {
	terr_t err		 = TW_SUCCESS;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;

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
	return err;
}