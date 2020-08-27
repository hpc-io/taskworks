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
	void *job;
	TWNATIVE_Job_t *jp;
	TWNATIVE_Task_t *tp;
	TWNATIVE_Event_t *evtp;

	cur_priority = 0;
	while (cur_priority < TWI_TASK_NUM_PRIORITY_LEVEL) {
		TWI_Nb_queue_pop (ep->queue[cur_priority], &job, &success);
		if (success) {
			jp = (TWNATIVE_Job_t *)job;
			// There can be garbage as task can be submitted to multiple queue at the same time
			if (jp->type == TWNATIVE_Job_type_task) {  // Decode as task
				tp		= (TWNATIVE_Task_t *)(jp->data);
				tp->job = NULL;
				TWNATIVE_Taski_run (tp, &success);
				if (success) break;
			} else {
				evtp	  = (TWNATIVE_Event_t *)(jp->data);
				evtp->job = NULL;
				TWNATIVE_Eventi_run (evtp, &success);
				if (success) break;
			}

			err = TWI_Disposer_dispose (TWNATIVEI_Disposer, job, TWI_Free);
			CHECK_ERR
		} else {  // No job in current queue, move to lower priority
			cur_priority++;
		}
	}
	if (successp) *successp = (cur_priority < TWI_TASK_NUM_PRIORITY_LEVEL) ? TWI_TRUE : TWI_FALSE;

err_out:;
	return err;
}

void *TWNATIVE_Engine_scheduler (void *data) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t locked;
	TWNATIVE_Thread_arg_t *ta = (TWNATIVE_Thread_arg_t *)data;

	OPA_incr_int (&(ta->ep->cur_nt));
	TWI_Disposer_join (TWNATIVEI_Disposer);

	// If number of thread decreases below our id, we quit
	while (ta->id < ta->ep->nt) {
		ta->ep->driver->sem.dec (ta->ep->njob);

		if (ta->ep->evt_driver) {
			TWI_Mutex_trylock (
				&(ta->ep->evt_lock),
				&locked);  // posix semaphore don't have dec and fect, so still need lock
			if (locked) {
				err = ta->ep->evt_driver->Loop_check_events (ta->ep->evt_loop, 100000);
				CHECK_ERR

				TWI_Mutex_unlock (&(ta->ep->evt_lock));

				ta->ep->driver->sem.inc (
					ta->ep->njob);	// Release another thread to check for events
				continue;
			}
		}

		TWNATIVE_Engine_scheduler_core (ta->ep, NULL);

	err_out:;
		if (err) { err = 0; }
	}

	TWI_Disposer_leave (TWNATIVEI_Disposer);
	OPA_decr_int (&(ta->ep->cur_nt));

	TWI_Free (ta);

	return NULL;
}