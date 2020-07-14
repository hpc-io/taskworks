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

#include "tw_err.h"

/* RW lock*/
#ifdef _WIN32
typedef SRWLOCK TWI_Rwlock_t;
#else
typedef pthread_rwlock_t TWI_Rwlock_t;
#endif
terr_t TWI_Rwlock_create (TWI_Rwlock_t *m);
terr_t TWI_Rwlock_free (TWI_Rwlock_t *m);
terr_t TWI_Rwlock_rlock (TWI_Rwlock_t *m);
terr_t TWI_Rwlock_wlock (TWI_Rwlock_t *m);
terr_t TWI_Rwlock_runlock (TWI_Rwlock_t *m);
terr_t TWI_Rwlock_wunlock (TWI_Rwlock_t *m);
