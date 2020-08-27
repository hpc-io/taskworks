/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Libevent driver implementation */

#include "twlibevt.h"

terr_t TWLIBEVT_Loop_create (TW_Handle_t *loop) {
	int err = TW_SUCCESS;
	struct event_config *config;
	TWLIBEVT_Loop_t *lp;

	lp = (TWLIBEVT_Loop_t *)TWI_Malloc (sizeof (TWLIBEVT_Loop_t));
	CHECK_PTR (lp);

	config	 = event_config_new ();
	lp->base = event_base_new_with_config (config);
	CHECK_LIBEVTPTR (lp->base);
	event_config_free (config);

	// List of commited events no managed by libevent
	lp->unmanaged_events = TWI_Ts_vector_create ();
	CHECK_PTR (lp->unmanaged_events);

	*loop = lp;

err_out:;
	return err;
}
terr_t TWLIBEVT_Loop_free (TW_Handle_t loop) {
	TWLIBEVT_Loop_t *lp = (TWLIBEVT_Loop_t *)loop;

	TWI_Ts_vector_free (lp->unmanaged_events);

	event_base_free (lp->base);

	TWI_Free (lp);

	return TW_SUCCESS;
}

terr_t TWLIBEVT_Loop_check_events (TW_Handle_t loop, ttime_t timeout) {
	terr_t err = TW_SUCCESS;
	ttime_t tstop;
	TWI_Bool_t have_evt;
	TWLIBEVT_Loop_t *lp = (TWLIBEVT_Loop_t *)loop;

	tstop = TWI_Time_now () + timeout;
	do {
		err = TWLIBEVTI_Check_for_single_event (lp, &have_evt);
		CHECK_ERR
		if (timeout == TW_ONCE) break;
	} while ((timeout == TW_TIMEOUT_NEVER || TWI_Time_now () < tstop) && have_evt);

err_out:;
	return err;
}
