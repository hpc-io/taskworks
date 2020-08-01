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
	int i;

	// Stop and free the ESs
	for (i = 0; i < ep->ness; i++) {
		abterr = ABT_xstream_join (ep->ess[i]);
		CHECK_ABTERR
		abterr = ABT_xstream_free (ep->ess + i);
		CHECK_ABTERR
	}
	TWI_Free (ep->ess);

	// Free the schedulars
	/* Scheduler freed automatically when ES get freed
	for (i = 0; i < ep->ness; i++) {
		// abterr = ABT_sched_free (ep->schedulers + i);
		// CHECK_ABTERR
	}
	*/
	TWI_Free (ep->schedulers);

	TWI_Nb_list_free (ep->tasks);

	// Pools freed automatically when all schedulers get freed
	////abterr = ABT_pool_free (&(ep->pool));

	if (ep->evt_loop) { ep->evt_driver->Loop_free (ep->evt_loop); }
	TWI_Mutex_finalize (&(ep->evt_lock));

	TWI_Free (ep);

err_out:;
	return err;
}