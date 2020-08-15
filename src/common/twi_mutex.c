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

/* Mutex wrapper for win32 and posix */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "taskworks_internal.h"

#ifdef TWI_DEBUG
#ifdef _WIN32
#define CHECK_MERR                                                          \
	{                                                                       \
		if (ret == WAIT_FAILED || ret == 0) PRINT_OS_ERR (GetLastError ()); \
		abort ();                                                           \
	}
#else

#define CHECK_MERR               \
	{                            \
		if (perr) {              \
			PRINT_OS_ERR (perr); \
			abort ();            \
		}                        \
	}
#endif
#else
#define CHECK_MERR \
	{}
#endif

TWI_Mutex_handle_t TWI_Mutex_create (void) {
	terr_t err			  = TW_SUCCESS;
	TWI_Mutex_handle_t mp = NULL;

	mp = (TWI_Mutex_handle_t)TWI_Malloc (sizeof (TWI_Mutex_t));
	CHECK_PTR (mp);

	err = TWI_Mutex_init (mp);
	CHECK_ERR

err_out:;
	if (err) {
		TWI_Free (mp);
		mp = NULL;
	}
	return mp;
}

void TWI_Mutex_free (TWI_Mutex_handle_t m) {
	TWI_Mutex_finalize (m);
	TWI_Free (m);
}

#ifdef _WIN32
terr_t TWI_Mutex_init (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	*m		   = CreateMutex (NULL, FALSE, NULL);
	if (*m == NULL) RET_OS_ERR (GetLastError ())
err_out:;
	return err;
}
void TWI_Mutex_finalize (TWI_Mutex_handle_t m) { CloseHandle (*m); }
void TWI_Mutex_lock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	DWORD ret;

	ret = WaitForSingleObject (*m, INFINITE);
	CHECK_MERR
}
void TWI_Mutex_trylock (TWI_Mutex_handle_t m, int *success) {
	terr_t err = TW_SUCCESS;
	DWORD ret;

	ret = WaitForSingleObject (*m, INFINITE);
	if (ret == WAIT_TIMEOUT)
		*success = 0;
	else {
		CHECK_MERR
		*success = 1;
	}
}
void TWI_Mutex_unlock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	BOOL ret;

	ret = ReleaseMutex (*m);
	CHECK_MERR
}
#else
terr_t TWI_Mutex_init (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_init (m, NULL);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
void TWI_Mutex_finalize (TWI_Mutex_handle_t m) {
	int perr;

	perr = pthread_mutex_destroy (m);
	CHECK_MERR
}
void TWI_Mutex_lock (TWI_Mutex_handle_t m) {
	int perr;

	DEBUG_PRINTF (3, "Try to lock %p\n", (void *)m);

	perr = pthread_mutex_lock (m);
	CHECK_MERR

	DEBUG_PRINTF (3, "Acquired lock %p\n", (void *)m);
}
void TWI_Mutex_trylock (TWI_Mutex_handle_t m, int *success) {
	int perr;

	perr = pthread_mutex_trylock (m);
	if (perr == EBUSY)
		*success = 0;
	else {
		CHECK_MERR
		*success = 1;
	}
}
void TWI_Mutex_unlock (TWI_Mutex_handle_t m) {
	int perr;

	// DEBUG_PRINTF (3, "Unlocking %p\n", (void *)m);

	perr = pthread_mutex_unlock (m);
	CHECK_MERR

	DEBUG_PRINTF (3, "Released lock %p\n", (void *)m);
}
#endif
