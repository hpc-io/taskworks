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

terr_t TWLIBEVT_Event_create (TW_Event_handler_t TWI_UNUSED evt_cb,
							  void TWI_UNUSED *evt_data,
							  TW_Event_args_t TWI_UNUSED arg,
							  TW_Handle_t TWI_UNUSED *event) {
	return TW_SUCCESS;
}
terr_t TWLIBEVT_Event_free (TW_Handle_t TWI_UNUSED event) { return TW_SUCCESS; }
terr_t TWLIBEVT_Event_commit (TW_Handle_t TWI_UNUSED event, TW_Handle_t TWI_UNUSED loop) {
	return TW_SUCCESS;
}
terr_t TWLIBEVT_Event_retract (TW_Handle_t TWI_UNUSED htask) { return TW_SUCCESS; }
