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
	int max_priority_level;
	TWI_Bool_t success;
	void *job;
	TWNATIVE_Job_t *jp;
	TWNATIVE_Task_t *tp;
	TWNATIVE_Event_t *evtp;
DEBUG
	if ((OPA_load_int (&(ep->sleep_nt)) > 0) && (ep->nt > 1)) {
		max_priority_level = TWI_TASK_NUM_PRIORITY_LEVEL;DEBUG
	} else {
		max_priority_level = TWI_TASK_NUM_PRIORITY_LEVEL - 1;DEBUG
	}
DEBUG
	cur_priority = 0;
	while (cur_priority < max_priority_level) {DEBUG
		TWI_Nb_queue_pop (ep->queue[cur_priority], &job, &success);
	DEBUG
		if (success) {DEBUG
			jp = (TWNATIVE_Job_t *)job;DEBUG
			// There can be garbage as task can be submitted to multiple queue at the same time
			if (jp->type == TWNATIVE_Job_type_task) {  // Decode as task
				tp		= (TWNATIVE_Task_t *)(jp->data);
				tp->job = NULL;DEBUG
				TWNATIVE_Taski_run (tp, &success);
				if (success) break;
			} else {
				evtp	  = (TWNATIVE_Event_t *)(jp->data);
				evtp->job = NULL;DEBUG
				TWNATIVE_Eventi_run (evtp, &success);
				if (success) break;
			}
			DEBUG
			err = TWI_Disposer_dispose (TWNATIVEI_Disposer, job, TWI_Free);
			CHECK_ERR
		} else {  // No job in current queue, move to lower priority
			cur_priority++;DEBUG
		}
	}DEBUG
	if (successp) *successp = (cur_priority < max_priority_level) ? TWI_TRUE : TWI_FALSE;

err_out:;
	return err;
}
#define THREAD_VAL_CHECK printf("%s:%s:%d: tid = %d: line %d: ta->id = %d, ta->ep->nt = %d\n", __FILE__, __func__, __LINE__, pthread_self(), __LINE__, ta->id, ta->ep->nt);
void *TWNATIVE_Engine_scheduler (void *data) {
	terr_t err = TW_SUCCESS;
	TWI_Bool_t locked;
	TWNATIVE_Thread_arg_t *ta = (TWNATIVE_Thread_arg_t *)data;

	OPA_incr_int (&(ta->ep->cur_nt));
	TWI_Disposer_join (TWNATIVEI_Disposer);

	// If number of thread decreases below our id, we quit
	while (ta->id < ta->ep->nt) {
	    DEBUG
		// Count # task and cmp it to # thread
		OPA_incr_int (&(ta->ep->sleep_nt));
		ta->ep->driver->sem.dec (ta->ep->njob);
		OPA_decr_int (&(ta->ep->sleep_nt));
		DEBUG THREAD_VAL_CHECK
		if (ta->ep->evt_driver) {
		    DEBUG THREAD_VAL_CHECK
		    printf("%s:%d:locked = %d\n", __func__, __LINE__, locked);
			TWI_Mutex_trylock (
				&(ta->ep->evt_lock),
				&locked);  // posix semaphore don't have dec and fect, so still need lock
		    THREAD_VAL_CHECK
		    DEBUG
		    if (locked) {
		        DEBUG
				err = ta->ep->evt_driver->Loop_check_events (ta->ep->evt_loop, 100000);
				CHECK_ERR
				THREAD_VAL_CHECK
				DEBUG
				TWI_Mutex_unlock (&(ta->ep->evt_lock));
	            printf("%s:%d:locked = %d, ta->ep->evt_lock = %d\n", __func__, __LINE__, locked, ta->ep->evt_lock);
				DEBUG
				ta->ep->driver->sem.inc (
					ta->ep->njob);	// Release another thread to check for events
				DEBUG
				continue;
			}
		}
		THREAD_VAL_CHECK
		DEBUG
		TWNATIVE_Engine_scheduler_core (ta->ep, NULL);
		THREAD_VAL_CHECK
	err_out:;
		if (err) {
			abort ();
			err = 0;
		}
	}
	DEBUG
	TWI_Disposer_leave (TWNATIVEI_Disposer);
	OPA_decr_int (&(ta->ep->cur_nt));

	TWI_Free (ta);

	return NULL;
}
