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

terr_t TWLIBEVTI_Check_for_single_event (TW_Handle_t TWI_UNUSED loop, TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	// int evterr;
	int i;
	int size;
	TWLIBEVT_Loop_t *lp = (TWLIBEVT_Loop_t *)loop;
	TWLIBEVT_Event_t *ep;

	*success = TWI_FALSE;

	TWI_Ts_vector_lock (lp->unmanaged_events);
	size = (int)TWI_Ts_vector_size (lp->unmanaged_events);
	for (i = 0; i < size; i++) {
		ep = (TWLIBEVT_Event_t *)lp->unmanaged_events->data[i];
		switch (ep->args.type) {
			case TW_Event_type_mpi:;
				/* code */
				break;
			case TW_Event_type_task:;
				/* code */
				break;
			case TW_Event_type_file:;
				/* fall through */
				TWI_FALL_THROUGH;
			case TW_Event_type_socket:;
				/* fall through */
				TWI_FALL_THROUGH;
			case TW_Event_type_timer:;
				/* fall through */
				TWI_FALL_THROUGH;
			default:;
				break;
		}
	}
	TWI_Ts_vector_unlock (lp->unmanaged_events);

	// err_out:;
	return err;
}
