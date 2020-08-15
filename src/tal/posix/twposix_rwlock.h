/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread thread driver rwlock */

#include "twi_mutex.h"
#include "twi_rwlock.h"
#include "twposix.h"

#if !(defined __USE_UNIX98 || defined __USE_XOPEN2K)
#define TWPOSIX_Rwlock_create	TWPOSIX_Mutex_create
#define TWPOSIX_Rwlock_free		TWPOSIX_Mutex_free
#define TWPOSIX_Rwlock_rlock	TWPOSIX_Mutex_lock
#define TWPOSIX_Rwlock_tryrlock TWPOSIX_Mutex_trylock
#define TWPOSIX_Rwlock_runlock	TWPOSIX_Mutex_unlock
#define TWPOSIX_Rwlock_wlock	TWPOSIX_Mutex_lock
#define TWPOSIX_Rwlock_trywlock TWPOSIX_Mutex_trylock
#define TWPOSIX_Rwlock_wunlock	TWPOSIX_Mutex_unlock
#endif

#define TWPOSIX_Rwlock_cb                                                                          \
	{                                                                                              \
		TWPOSIX_Rwlock_create, TWPOSIX_Rwlock_free, TWPOSIX_Rwlock_rlock, TWPOSIX_Rwlock_tryrlock, \
			TWPOSIX_Rwlock_runlock, TWPOSIX_Rwlock_wlock, TWPOSIX_Rwlock_trywlock,                 \
			TWPOSIX_Rwlock_wunlock                                                                 \
	}

#if defined __USE_UNIX98 || defined __USE_XOPEN2K
terr_t TWPOSIX_Rwlock_create (TW_handle_t *m);
void TWPOSIX_Rwlock_free (TW_handle_t m);
void TWPOSIX_Rwlock_rlock (TW_handle_t m);
void TWPOSIX_Rwlock_tryrlock (TW_handle_t m, TWI_Bool_t *success);
void TWPOSIX_Rwlock_runlock (TW_handle_t m);
void TWPOSIX_Rwlock_wlock (TW_handle_t m);
void TWPOSIX_Rwlock_trywlock (TW_handle_t m, TWI_Bool_t *success);
void TWPOSIX_Rwlock_wunlock (TW_handle_t m);
#endif