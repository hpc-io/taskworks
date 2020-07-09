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

terr_t TWABT_Event_create (TW_Event_handler_t evt_cb,
						   void *evt_data,
						   TW_Event_attr_t attr,
						   TW_Handle_t *hevt) {}  // Create a new event
terr_t TWABT_Event_free (TW_Handle_t hevt) {}
terr_t TWABT_Event_commit (TW_Handle_t engine, TW_Handle_t hevt) {}	 // Commit event, start watching
terr_t TWABT_Event_retract (TW_Handle_t hevt) {}					 // Stop watching