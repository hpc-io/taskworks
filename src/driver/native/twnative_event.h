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

#include "twnative.h"

typedef struct TWNATIVE_Event_t {
	TW_Event_handler_t handler;
	void *data;
	TW_Event_driver_handle_t driver;
	OPA_int_t status;  // Status of the event
	void *driver_obj;
	void *dispatcher_obj;
	TWNATIVE_Engine_t *eng;
	struct TWNATIVE_Event_t **native_task_ctx;
	TW_Event_args_t arg;
	TWNATIVE_Job_t *job;
	int repeat;
	// NATIVE_task native_task;	// Argobot task handle
} TWNATIVE_Event_t;

/* Event callbacks */
terr_t TWNATIVE_Event_create (TW_Event_handler_t evt_cb,
							  void *evt_data,
							  TW_Event_args_t args,
							  void *dispatcher_obj,
							  TW_Handle_t *hevt);  // Create a new event
terr_t TWNATIVE_Event_free (TW_Handle_t hevt);
terr_t TWNATIVE_Event_commit (TW_Handle_t hevt,
							  TW_Handle_t engine);	// Commit event, start watching
terr_t TWNATIVE_Event_retract (TW_Handle_t hevt);	// Stop watching

/* Event internal functions */
void TWNATIVE_Eventi_free (void *hevt);
terr_t TWNATIVE_Eventi_cb (TW_Event_args_t *arg, void *data);
terr_t TWNATIVE_Eventi_update_status (TWNATIVE_Event_t *ep,
									  int old_stat,
									  int new_stat,
									  TWI_Bool_t *successp);
void TWNATIVE_Eventi_task_cb (void *data);
void TWNATIVE_Eventi_task_run (TWNATIVE_Event_t *ep, TWI_Bool_t *successp);