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

terr_t TWABT_Event_create (TW_Event_handler_t TWI_UNUSED evt_cb,
						   void TWI_UNUSED *evt_data,
						   TW_Event_args_t TWI_UNUSED attr,
						   void TWI_UNUSED *dispatcher_obj,
						   TW_Handle_t TWI_UNUSED *hevt) {
	return TW_ERR_NOT_SUPPORTED;
}  // Create a new event
terr_t TWABT_Event_free (TW_Handle_t TWI_UNUSED hevt) { return TW_ERR_NOT_SUPPORTED; }
terr_t TWABT_Event_commit (TW_Handle_t TWI_UNUSED engine, TW_Handle_t TWI_UNUSED hevt) {
	return TW_ERR_NOT_SUPPORTED;
}  // Commit event, start watching
terr_t TWABT_Event_retract (TW_Handle_t TWI_UNUSED hevt) {
	return TW_ERR_NOT_SUPPORTED;
}  // Stop watching