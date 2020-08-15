/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread thread driver mutex */

#pragma once

#include <pthread.h>

#include "twi_mutex.h"
#include "twposix.h"

#define TWPOSIX_Mutex_cb                                                                     \
	{                                                                                        \
		TWPOSIX_Mutex_create, TWPOSIX_Mutex_free, TWPOSIX_Mutex_lock, TWPOSIX_Mutex_trylock, \
			TWPOSIX_Mutex_unlock                                                             \
	}

terr_t TWPOSIX_Mutex_create (TW_handle_t *m);
void TWPOSIX_Mutex_free (TW_handle_t m);
void TWPOSIX_Mutex_lock (TW_handle_t m);
void TWPOSIX_Mutex_trylock (TW_handle_t m, TWI_Bool_t *success);
void TWPOSIX_Mutex_unlock (TW_handle_t m);
