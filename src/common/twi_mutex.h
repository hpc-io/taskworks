/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Cross-platform read write lock wrapper */

#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "taskworks_internal.h"

#ifdef _WIN32
typedef HANDLE TWI_Mutex_t;
#else
typedef pthread_mutex_t TWI_Mutex_t;
#endif
typedef TWI_Mutex_t *TWI_Mutex_handle_t;

terr_t TWI_Mutex_create (TWI_Mutex_handle_t *m);
terr_t TWI_Mutex_free (TWI_Mutex_handle_t m);
terr_t TWI_Mutex_init (TWI_Mutex_handle_t m);
terr_t TWI_Mutex_finalize (TWI_Mutex_handle_t m);
terr_t TWI_Mutex_lock (TWI_Mutex_handle_t m);
terr_t TWI_Mutex_trylock (TWI_Mutex_handle_t m, int *success);
terr_t TWI_Mutex_unlock (TWI_Mutex_handle_t m);
