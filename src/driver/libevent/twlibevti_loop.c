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

terr_t TWLIBEVTI_Check_for_single_unmanaged_event (TWLIBEVT_Loop_t *lp, TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	int i;
	int size;
	TWLIBEVT_Event_t *ep = NULL;

	*success = TWI_FALSE;

	TWI_Ts_vector_lock (lp->unmanaged_events);
	size = (int)TWI_Ts_vector_size (lp->unmanaged_events);
	for (i = 0; i < size; i++) {
		ep = (TWLIBEVT_Event_t *)lp->unmanaged_events->data[i];
#ifdef HAVE_MPI
		if (ep->args.type == TW_Event_type_mpi) {
			int mpierr;
			int flag;
			MPI_Status status;

			mpierr = MPI_Test (&(ep->args.args.mpi.req), &flag, &status);
			CHECK_MPIERR

			if (flag) {
				TWLIBEVTI_Evt_mpi_cb (ep, flag, status);
				break;
			}
		}
#endif
	}
	TWI_Ts_vector_unlock (lp->unmanaged_events);

	// Remove from event queue
	if (ep && i < size) { TWI_Ts_vector_swap_erase (lp->unmanaged_events, ep); }

err_out:;

	return err;
}

terr_t TWLIBEVTI_Check_for_single_event (TW_Handle_t loop, TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	int evterr;
	TWLIBEVT_Loop_t *lp = (TWLIBEVT_Loop_t *)loop;

	err = TWLIBEVTI_Check_for_single_unmanaged_event (lp, success);
	CHECK_ERR

	if (!(*success)) {	// No unmanagemen
		evterr = event_base_loop (lp->base, EVLOOP_ONCE | EVLOOP_NONBLOCK);
		if (evterr == 0) {
			*success = TWI_TRUE;
		} else if (evterr == 1) {
			*success = TWI_FALSE;
		} else {
			CHECK_LIBEVTERR
		}
	}

err_out:;
	return err;
}
