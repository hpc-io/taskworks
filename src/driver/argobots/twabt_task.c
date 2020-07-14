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
						  TW_Handle_t *htask) {	 // Create a new task
	terr_t err;
	int abterr;
	TWABT_Task_t *tp;

	tp = (TWABT_Task_t *)TWI_Malloc (sizeof (TWABT_Task_t));
	CHK_PTR (tp)

	tp->handler		= task_cb;
	tp->data		= task_data;
	tp->dep_cb		= dep_cb;
	tp->dep_stat_cb = dep_stat_cb;
	tp->dep_stat	= NULL;
	tp->priority	= 0;
	tp->ep			= NULL;
	tp->abt_task	= ABT_TASK_NULL;

	memset (&(tp->parents), 0, sizeof (tp->parents));
	memset (&(tp->childs), 0, sizeof (tp->childs));

	err = TWI_List_create (&(tp->parents));
	CHK_ERR
	err = TWI_List_create (&(tp->childs));
	CHK_ERR

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
	terr_t err;
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
	TWI_List_free (&(tp->parents));
	TWI_List_free (&(tp->childs));
	TWI_Free (tp);
	return err;
}

terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  TW_Handle_t *htask) {	 // Create a new barrier task
}

terr_t TWABT_Task_commit (TW_Handle_t htask, TW_Handle_t engine) {	// Put the task into the dag
	terr_t err;
	int abterr;
	int i, j, s;
	TWABT_Engine_t *ep = (TWABT_Task_t *)engine;
	TWABT_Task_t *tp   = (TWABT_Task_t *)htask;
	TWABT_Task_t *itr;

	err = TWABTI_Task_queue (tp);
	CHK_ERR

err_out:;
	return err;
}
terr_t TWABT_Task_retract (TW_Handle_t htask) {}	  // Remove task form the dag
terr_t TWABT_Task_wait_single (TW_Handle_t htask) {}  // Wait for a single task to complete. The
													  // calling thread joins the worker on the
													  // job being waited and all its parents.
terr_t TWABT_Task_wait (TW_Handle_t *htasks, int *num_tasks, ttime_t timeout) {
}  // Wait for a multiple task to complete.
terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent) {}
terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent) {}
terr_t TWABT_Task_inq (TW_Handle_t htask, TW_Task_inq_type_t inqtype, int *ret) {}