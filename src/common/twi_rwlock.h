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
#if defined __USE_UNIX98 || defined __USE_XOPEN2K
typedef pthread_rwlock_t TWI_Rwlock_t;
#else
typedef pthread_mutex_t TWI_Rwlock_t;
#endif
#endif

typedef TWI_Rwlock_t *TWI_Rwlock_handle_t;

#if defined __USE_UNIX98 || defined __USE_XOPEN2K || defined _WIN32
terr_t TWI_Rwlock_create (TWI_Rwlock_handle_t *l);
terr_t TWI_Rwlock_free (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_init (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_finalize (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_rlock (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_wlock (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_runlock (TWI_Rwlock_handle_t l);
terr_t TWI_Rwlock_wunlock (TWI_Rwlock_handle_t l);
#else
#define TWI_Rwlock_create	TWI_Mutex_create
#define TWI_Rwlock_free		TWI_Mutex_free
#define TWI_Rwlock_init		TWI_Mutex_init
#define TWI_Rwlock_finalize TWI_Mutex_finalize
#define TWI_Rwlock_rlock	TWI_Mutex_lock
#define TWI_Rwlock_wlock	TWI_Mutex_lock
#define TWI_Rwlock_runlock	TWI_Mutex_unlock
#define TWI_Rwlock_wunlock	TWI_Mutex_unlock
#endif