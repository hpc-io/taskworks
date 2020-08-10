/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver functions */

#pragma once

#include "twabt.h"

typedef struct TWABT_Event_t {
	TW_Event_handler_t handler;
	void *data;
	TW_Event_driver_handle_t driver;
	OPA_int_t status;  // Status of the event
	void *driver_obj;
	void *dispatcher_obj;
	TWABT_Engine_t *eng;
	struct TWABT_Event_t **abt_task_ctx;
	TW_Event_args_t *arg;
	int repeat;
	// ABT_task abt_task;	// Argobot task handle
} TWABT_Event_t;

/* Event callbacks */
terr_t TWABT_Event_create (TW_Event_handler_t evt_cb,
						   void *evt_data,
						   TW_Event_args_t args,
						   void *dispatcher_obj,
						   TW_Handle_t *hevt);	// Create a new event
terr_t TWABT_Event_free (TW_Handle_t hevt);
terr_t TWABT_Event_commit (TW_Handle_t hevt, TW_Handle_t engine);  // Commit event, start watching
terr_t TWABT_Event_retract (TW_Handle_t hevt);					   // Stop watching

/* Event internal functions */
void TWABTI_Event_free (void *hevt);
terr_t TWABTI_Event_cb (TW_Event_args_t *arg, void *data);
terr_t TWABTI_Event_update_status (TWABT_Event_t *ep,
								   int old_stat,
								   int new_stat,
								   TWI_Bool_t *successp);
void TWABTI_Event_task_cb (void *data);
void TWABTI_Event_task_run (TWABT_Event_t *ep, TWI_Bool_t *successp);