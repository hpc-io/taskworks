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

terr_t TWABTI_Engine_free (TWABT_Engine_t *ep) {
	terr_t err = TW_SUCCESS;
	int abterr;
	TWI_Nb_list_itr_t j;
	int i;

	// Stop and free the ESs
	for (i = 0; i < ep->ness; i++) {
		abterr = ABT_xstream_join (ep->ess[i]);
		CHECK_ABTERR
		abterr = ABT_xstream_free (ep->ess + i);
		CHECK_ABTERR
	}

	// Free the schedulars
	/* Scheduler freed automatically when ES get freed
	for (i = 0; i < ep->ness; i++) {
		// abterr = ABT_sched_free (ep->schedulers + i);
		// CHECK_ABTERR
	}
	*/

	// Retract all tasks
	TWI_Nb_list_inc_ref (ep->tasks);
	for (j = TWI_Nb_list_begin (ep->tasks); j != TWI_Nb_list_end (ep->tasks);
		 j = TWI_Nb_list_next (j)) {
		err = TWABT_Task_retract ((TWABT_Task_t *)j->data);
		CHECK_ERR
	}
	TWI_Nb_list_dec_ref (ep->tasks);

	// Pools freed automatically when all schedulers get freed
	////abterr = ABT_pool_free (&(ep->pool));

	if (ep->evt_loop) { ep->evt_driver->Loop_free (ep->evt_loop); }

	err = TWI_Disposer_dispose (TWABTI_Disposer, (void *)ep, TWABTI_Engine_free_core);
	CHECK_ERR

err_out:;
	return err;
}

void TWABTI_Engine_free_core (void *obj) {
	TWABT_Engine_t *ep = (TWABT_Engine_t *)obj;

	TWI_Free (ep->ess);
	TWI_Free (ep->schedulers);
	TWI_Nb_list_free (ep->tasks);
	TWI_Mutex_finalize (&(ep->evt_lock));
	TWI_Free (ep);
}