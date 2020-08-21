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

#include "driver.h"
#include "tal.h"
#include "taskworks_internal.h"
#include "twnative_engine.h"
#include "twnative_event.h"
#include "twnative_task.h"

typedef enum TWNATIVEI_Thread_driver_t {
	TWNATIVEI_Thread_driver_default,
#ifdef _WIN32
	TWNATIVEI_Thread_driver_win32
#else
	TWNATIVEI_Thread_driver_pthread
#endif
} TWNATIVEI_Thread_driver_t;

extern TWI_Disposer_handle_t TWNATIVEI_Disposer;

extern TWI_Ts_vector_handle_t TWNATIVEI_Tasks;
extern TWI_Ts_vector_handle_t TWNATIVEI_Engines;
extern TWI_Ts_vector_handle_t TWNATIVEI_Events;

extern TW_Thread_driver_handle_t TWNATIVEI_Driver;

/* Init callbacks */
terr_t TWNATIVE_Init (int *argc, char ***argv);
terr_t TWNATIVE_Finalize (void);

/* Schedulers */
