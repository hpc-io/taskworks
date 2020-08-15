/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,
 *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Rwlock wrapper for win32 and posix */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "taskworks_internal.h"

#if !defined _WIN32 && (defined __USE_UNIX98 || defined __USE_XOPEN2K)
#ifdef TWI_DEBUG
#define CHECK_MERR               \
	{                            \
		if (perr) {              \
			PRINT_OS_ERR (perr); \
			abort ();            \
		}                        \
	}
#else
#define CHECK_MERR \
	{}
#endif
#endif

TWI_Rwlock_handle_t TWI_Rwlock_create (void) {
	terr_t err			  = TW_SUCCESS;
	TWI_Rwlock_handle_t l = NULL;

	l = TWI_Malloc (sizeof (TWI_Rwlock_t));
	CHECK_PTR (l);

	err = TWI_Rwlock_init (l);

err_out:;
	if (err) {
		TWI_Free (l);
		l = NULL;
	}
	return l;
}

void TWI_Rwlock_free (TWI_Rwlock_handle_t l) {
	TWI_Rwlock_finalize (l);
	TWI_Free (l);
}

#ifdef _WIN32
terr_t TWI_Rwlock_init (TWI_Rwlock_handle_t l) {
	InitializeSRWLock (l);

	return TW_SUCCESS;
}
void TWI_Rwlock_finalize (TWI_Rwlock_handle_t l) { return TW_SUCCESS; }
void TWI_Rwlock_rlock (TWI_Rwlock_handle_t l) { AcquireSRWLockShared (l); }

void TWI_Rwlock_tryrlock (TWI_Rwlock_handle_t l, int *success) {
	BOOLEAN ret;

	ret = TryAcquireSRWLockShared (l);

	if (ret == TRUE)
		*success = 1;
	else
		*success = 0;
}
void TWI_Rwlock_wlock (TWI_Rwlock_handle_t l) { AcquireSRWLockExclusive (l); }
void TWI_Rwlock_trywlock (TWI_Rwlock_handle_t l, int *success) {
	BOOLEAN ret;

	ret = TryAcquireSRWLockExclusive (l);

	if (ret == TRUE)
		*success = 1;
	else
		*success = 0;
}
void TWI_Rwlock_runlock (TWI_Rwlock_handle_t l) { ReleaseSRWLockShared (l); }
void TWI_Rwlock_wunlock (TWI_Rwlock_handle_t l) { ReleaseSRWLockExclusive (l); }
#elif defined __USE_UNIX98 || defined __USE_XOPEN2K
terr_t TWI_Rwlock_init (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_init (l, NULL);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
void TWI_Rwlock_finalize (TWI_Rwlock_handle_t l) {
	int perr;

	perr = pthread_rwlock_destroy (l);
	CHECK_MERR
}
terr_t TWI_Rwlock_rlock (TWI_Rwlock_handle_t l) {
	int perr;

	perr = pthread_rwlock_rdlock (l);
	CHECK_MERR
}
void TWI_Rwlock_tryrdlock (TWI_Rwlock_handle_t l, int *success) {
	int perr;

	perr = pthread_rwlock_tryrdlock (l);
	if (perr == EBUSY)
		*success = 0;
	else {
		CHECK_MERR
		*success = 1;
	}
}
void TWI_Rwlock_wlock (TWI_Rwlock_handle_t l) {
	int perr;

	perr = pthread_rwlock_wrlock (l);
	CHECK_MERR
}
void TWI_Rwlock_trywrlock (TWI_Rwlock_handle_t l, int *success) {
	int perr;

	perr = pthread_rwlock_trywrlock (l);
	if (perr == EBUSY)
		*success = 0;
	else {
		CHECK_MERR
		*success = 1;
	}
}
void TWI_Rwlock_runlock (TWI_Rwlock_handle_t l) {
	int perr;

	perr = pthread_rwlock_unlock (l);
	CHECK_MERR
}
void TWI_Rwlock_wunlock (TWI_Rwlock_handle_t l) {
	int perr;

	perr = pthread_rwlock_unlock (l);
	CHECK_MERR
}
#endif
