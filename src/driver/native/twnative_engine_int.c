/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Native driver engine internal fuinctions */

#include "twnative.h"

terr_t TWNATIVE_Enginei_free (TWNATIVE_Engine_t *ep) {
	terr_t err = TW_SUCCESS;
	int i;
	int nt;
	TWI_Bool_t have_job;
	TWNATIVE_Job_t *jp;

	// Stop all threads
	nt	   = ep->nt;
	ep->nt = 0;	 // Set num worker to 0
	// Wake up threads
	for (i = 0; i < nt; i++) { ep->driver->sem.inc (ep->njob); }
	// Join threads
	for (i = 0; i < nt; i++) {
		err = ep->driver->join (ep->threads[i], NULL);
		CHECK_ERR
	}

	// Retract all tasks
	// All commited tasks are either in the queue or connected to queued task via dependency
	for (i = 0; i < TWI_TASK_NUM_PRIORITY_LEVEL; i++) {
		TWI_Nb_queue_pop (ep->queue[i], (void *)(&jp), &have_job);
		while (have_job) {
			if (jp->type == TWNATIVE_Job_type_task && jp->data) {
				err = TWNATIVE_Enginei_retract_task_r (ep, jp->data);
				CHECK_ERR
			}
			TWI_Disposer_dispose (TWNATIVEI_Disposer, jp, TWI_Free);
			TWI_Nb_queue_pop (ep->queue[i], (void *)(&jp), &have_job);
		}
	}

	TWI_Disposer_dispose (TWNATIVEI_Disposer, ep, TWNATIVE_Enginei_free_core);

err_out:;
	return err;
}

void TWNATIVE_Enginei_free_core (void *obj) {
	TWNATIVE_Engine_t *ep = (TWNATIVE_Engine_t *)obj;

	TWI_Free (ep->threads);
	TWI_Free (ep);
}

terr_t TWNATIVE_Enginei_retract_task_r (TWNATIVE_Engine_t *ep, TW_Handle_t task) {
	terr_t err = TW_SUCCESS;
	TWI_Vector_t s;
	TWI_Ts_vector_itr_t i;
	TWNATIVE_Task_t *tp, *cp;
	TWNATIVE_Task_dep_t *dp;

	err = TWI_Vector_init (&s);
	CHECK_ERR

	TWI_Vector_push_back (&s, task);

	while (TWI_Vector_size (&s) > 0) {
		TWI_Vector_pop_back (&s, (void **)&tp);

		// Push dependent tasks
		for (i = TWI_Ts_vector_begin (tp->childs); i != TWI_Ts_vector_end (tp->childs); i++) {
			dp = *i;
			cp = (TWNATIVE_Task_t *)OPA_load_ptr (&(dp->child));
			if (cp && cp->ep == ep) { TWI_Vector_push_back (&s, cp); }
		}

		// Retract task
		err = TWNATIVE_Task_retract (tp);
		CHECK_ERR
	}

err_out:;
	return err;
}