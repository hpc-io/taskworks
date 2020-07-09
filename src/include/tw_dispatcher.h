/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Function that does not involve drivers */

#pragma once

#include "taskworks.h"

#define TW_TIMEOUT_NEVER -1LL

#define TW_HANDLE_NULL NULL

typedef int terr_t;

typedef enum TW_Backend_t {
	TW_Backend_default,
	TW_Backend_argobots,
	TW_Backend_native
} TW_Backend_t;

typedef enum TW_Event_backend_t {
	TW_Event_backend_default,
	TW_Event_backend_libevent,
	TW_Event_backend_none
} TW_Event_backend_t;

// Only one backend, given at init
// Add env option, highest priority
// Provide default (NULL)
terr_t TW_Init (TW_Backend_t backend, TW_Event_backend_t evt_backend, int *argc, char ***argv);
terr_t TW_Finalize ();

const char *TW_Get_err_msg (terr_t err);