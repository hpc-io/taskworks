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

terr_t TWNATIVE_Engine_scheduler_core (TWNATIVE_Engine_t *ep, TWI_Bool_t *successp) {
	terr_t err = TW_SUCCESS;
	int cur_priority;
	TWI_Bool_t success;
	void *task;
	TWNATIVE_Task_t *tp;

	cur_priority = 0;
	while (cur_priority < TWI_TASK_NUM_PRIORITY_LEVEL) {
		TWI_Nb_queue_pop (ep->queue[cur_priority], &task, &success);
		if (success) {
			tp = *((TWNATIVE_Task_t **)task);

			err = TWI_Disposer_dispose (TWNATIVEI_Disposer, task, TWI_Free);
			CHECK_ERR

			if (tp) {  // There can be garbage as task can be submitted to multiple queue at the
					   // same time
				*(tp->self) = NULL;
				TWNATIVE_Taski_run (tp, &success);
				if (success) break;
			}
		} else {  // No job in current queue, move to lower priority
			cur_priority++;
		}
	}
	if (successp) *successp = (cur_priority < TWI_TASK_NUM_PRIORITY_LEVEL) ? TWI_TRUE : TWI_FALSE;

err_out:;
	return err;
}

void *TWNATIVE_Engine_scheduler (void *data) {
	TWNATIVE_Thread_arg_t *ta = (TWNATIVE_Thread_arg_t *)data;

	OPA_incr_int (&(ta->ep->cur_nt));
	TWI_Disposer_join (TWNATIVEI_Disposer);

	// If number of thread decreases below our id, we quit
	while (ta->id < ta->ep->nt) {
		ta->ep->driver->sem.dec (ta->ep->njob);
		TWNATIVE_Engine_scheduler_core (ta->ep, NULL);
	}

	TWI_Disposer_leave (TWNATIVEI_Disposer);
	OPA_decr_int (&(ta->ep->cur_nt));

	TWI_Free (ta);

	return NULL;
}