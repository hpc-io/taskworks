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

// TODO: Remove macro, just expose the same name
#ifdef _WIN32
terr_t TWI_Mutex_create (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	*m		   = CreateMutex (NULL, FALSE, NULL);
	if (*m == NULL) RET_OS_ERR (GetLastError ())
err_out:;
	return err;
}
terr_t TWI_Mutex_free (TWI_Mutex_handle_t m) {
	CloseHandle (*m);
	return TW_SUCCESS;
}
terr_t TWI_Mutex_lock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	DWORD ret;

	ret = WaitForSingleObject (*m, INFINITE);
	if (ret == WAIT_FAILED) RET_OS_ERR (GetLastError ())
err_out:;
	return err;
}
terr_t TWI_Mutex_trylock (TWI_Mutex_handle_t m, int *success) {
	terr_t err = TW_SUCCESS;
	DWORD ret;

	ret = WaitForSingleObject (*m, INFINITE);
	if (ret == WAIT_FAILED) RET_OS_ERR (GetLastError ())

	if (ret == WAIT_TIMEOUT)
		*success = 0;
	else
		*success = 1;

err_out:;
	return err;
}
terr_t TWI_Mutex_unlock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	BOOL ret;

	ret = ReleaseMutex (*m);
	if (ret == 0) RET_OS_ERR (GetLastError ())

err_out:;
	return err;
}
#else
terr_t TWI_Mutex_create (TWI_Mutex_handle_t *m) {
	terr_t err = TW_SUCCESS;
	TWI_Mutex_handle_t mp;

	mp = (TWI_Mutex_handle_t)TWI_Malloc (sizeof (TWI_Mutex_t));
	CHECK_PTR (mp);

	err = TWI_Mutex_init (mp);
	CHECK_ERR

	*m = mp;

err_out:;
	if (err) { TWI_Free (mp); }
	return err;
}
terr_t TWI_Mutex_free (TWI_Mutex_handle_t m) {
	terr_t err;

	err = TWI_Mutex_finalize (m);
	TWI_Free (m);

	return err;
}
terr_t TWI_Mutex_init (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_init (m, NULL);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Mutex_finalize (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_destroy (m);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Mutex_lock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_lock (m);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Mutex_trylock (TWI_Mutex_handle_t m, int *success) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_trylock (m);
	if (perr) {
		if (perr == EBUSY)
			*success = 0;
		else
			RET_OS_ERR (perr)
	} else
		*success = 1;

err_out:;
	return err;
}
terr_t TWI_Mutex_unlock (TWI_Mutex_handle_t m) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_mutex_unlock (m);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
#endif
