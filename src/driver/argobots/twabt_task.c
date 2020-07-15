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
						  TW_Task_dep_handler_t dep_cb,
						  TW_Task_dep_stat_handler_t dep_stat_cb,
						  int tag,
						  void *dispatcher_obj,
						  TW_Handle_t *htask) {	 // Create a new task
	terr_t err = TW_SUCCESS;
	int abterr;
	TWABT_Task_t *tp;

	tp = (TWABT_Task_t *)TWI_Malloc (sizeof (TWABT_Task_t));
	CHK_PTR (tp)
	tp->parents = NULL;
	tp->childs	= NULL;
	err			= TWI_List_create (&(tp->parents));
	CHK_ERR
	err = TWI_List_create (&(tp->childs));
	CHK_ERR
	tp->handler		   = task_cb;
	tp->data		   = task_data;
	tp->dep_cb		   = dep_cb;
	tp->dep_stat_cb	   = dep_stat_cb;
	tp->tag			   = tag;
	tp->dep_stat	   = NULL;
	tp->priority	   = 0;
	tp->ep			   = NULL;
	tp->abt_task	   = ABT_TASK_NULL;
	tp->dispatcher_obj = dispatcher_obj;
	OPA_store_int (&(tp->status), TW_Task_STAT_PENDING);

	*htask = tp;

err_out:;
	if (err) {
		if (tp) {
			TWI_List_free (tp->parents);
			TWI_List_free (tp->childs);
			TWI_Free (tp);
		}
	}
	return err;
}

terr_t TWABT_Task_free (TW_Handle_t htask) {  // Free up a task
	terr_t err = TW_SUCCESS;
	int abterr;
	int i, j, s;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;
	TWABT_Task_dep_t *dp;
	TWI_List_itr_t itr;

	// Remove all dependencies
	itr = TWI_List_head (tp->childs);  // Childs
	while (itr) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		j = OPA_decr_and_test_int (&(dp->ref));
		if (!j) { TWI_Free (dp); }

		itr = TWI_List_next (itr);
	}
	itr = TWI_List_head (tp->parents);	// Parents
	while (itr) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		j = OPA_decr_and_test_int (&(dp->ref));
		if (!j) { TWI_Free (dp); }

		itr = TWI_List_next (itr);
	}

	if (tp->abt_task != ABT_TASK_NULL) { ABT_task_free (&(tp->abt_task)); }

err_out:;
	TWI_List_free (tp->parents);
	TWI_List_free (tp->childs);
	TWI_Free (tp);
	return err;
}

terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  TW_Handle_t *htask) {	 // Create a new barrier task
}

terr_t TWABT_Task_commit (TW_Handle_t htask, TW_Handle_t engine) {	// Put the task into the dag
	terr_t err = TW_SUCCESS;
	int abterr;
	int i, j, s;
	TWABT_Engine_t *ep = (TWABT_Engine_t *)engine;
	TWABT_Task_t *tp   = (TWABT_Task_t *)htask;
	TWABT_Task_t *itr;

	if (OPA_cas_int (&(tp->status), TW_Task_STAT_PENDING, TW_Task_STAT_WAITING) !=
		TW_Task_STAT_PENDING) {
		RET_ERR (TW_ERR_STATUS)
	}

	tp->ep = ep;

	err = TWABTI_Task_queue (tp);
	CHK_ERR

err_out:;
	return err;
}
terr_t TWABT_Task_retract (TW_Handle_t htask) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i, j, s;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;
	TWABT_Task_t *itr;

	err = TWABTI_Task_queue (tp);
	CHK_ERR

err_out:;
	return err;
}

// Wait for a single task to complete. The
// calling thread joins the worker on the
// job being waited and all its parents.
terr_t TWABT_Task_wait_single (TW_Handle_t htask, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int abterr;
	ttime_t stoptime;
	ABT_task_state stat;
	TWABT_Task_t *tp = (TWABT_Task_t *)htask;
	TWABT_Task_t *itr;

	if (timeout == TW_TIMEOUT_NEVER) {
		abterr = ABT_task_join (tp->abt_task);
		CHECK_ABTERR
	} else {
		stoptime = TWI_Time_now () + timeout;
		while (TWI_Time_now () < stoptime) {
			// TODO: join the workers
			abterr = ABT_task_get_state (tp->abt_task, &stat);
			if (stat == ABT_TASK_STATE_TERMINATED) { break; }
		}
		if (stat != ABT_TASK_STATE_TERMINATED) { RET_ERR (TW_ERR_TIMEOUT) }
	}
err_out:;
	return err;
}

// Wait for a multiple task to complete.
terr_t TWABT_Task_wait (TW_Handle_t *htasks, int num_tasks, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	int i;
	ttime_t stoptime, now;
	TWABT_Task_t *tp;

	if (timeout == TW_TIMEOUT_NEVER) {
		for (i = 0; i < num_tasks; i++) {
			err = TWABT_Task_wait_single (htasks[i], timeout);
			CHK_ERR
		}
	} else {
		now		 = TWI_Time_now ();
		stoptime = now + timeout;
		for (i = 0; i < num_tasks && now < stoptime; i++) {
			err = TWABT_Task_wait_single (htasks[i], stoptime - now);
			CHK_ERR
			now = TWI_Time_now ();
		}
		if (i < num_tasks) { RET_ERR (TW_ERR_TIMEOUT) }
	}
err_out:;
	return err;
}

terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err = TW_SUCCESS;
	int i, is_zero;
	TWABT_Task_t *cp = (TWABT_Task_t *)child;
	TWABT_Task_t *pp = (TWABT_Task_t *)parent;
	TWABT_Task_dep_t *dp;
	TWI_List_itr_t itr;

	dp = (TWABT_Task_dep_t *)TWI_Malloc (sizeof (TWABT_Task_dep_t));
	CHK_PTR (dp)

	dp->parent = pp;
	dp->child  = cp;
	OPA_store_int (&(dp->ref), 2);
	OPA_store_int (&(dp->status), TW_Task_STAT_PENDING);

	// Remove all dependencies
	itr = TWI_List_head (cp->parents);	// Parents
	while (itr) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		if (dp->parent == pp) {
			is_zero = OPA_decr_and_test_int (&(dp->ref));
			if (is_zero) { TWI_Free (dp); }
			break;
		}

		itr = TWI_List_next (itr);
	}
	if (!itr) { RET_ERR (TW_ERR_NOT_FOUND) }

err_out:;

	return err;
}
terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent) {
	terr_t err = TW_SUCCESS;
	int i, is_zero;
	TWABT_Task_t *cp = (TWABT_Task_t *)child;
	TWABT_Task_t *pp = (TWABT_Task_t *)parent;
	TWABT_Task_dep_t *dp;
	TWI_List_itr_t itr;

	// Remove all dependencies
	itr = TWI_List_head (cp->parents);	// Parents
	while (itr) {
		dp = itr->data;

		// Decrease ref count, if we are the last one, free the dep struct
		if (dp->parent == pp) {
			is_zero = OPA_decr_and_test_int (&(dp->ref));
			if (is_zero) { TWI_Free (dp); }
			break;
		}

		itr = TWI_List_next (itr);
	}
	if (!itr) { RET_ERR (TW_ERR_NOT_FOUND) }

err_out:;

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
			RET_ERR (TW_ERR_INVAL);
			break;
	}

err_out:;
	return err;
}