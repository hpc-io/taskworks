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

terr_t TWI_Rwlock_create (TWI_Rwlock_handle_t *l) {
	terr_t err = TW_SUCCESS;

	*l = TWI_Malloc (sizeof (TWI_Rwlock_t));
	CHECK_PTR (*l);

	err = TWI_Rwlock_init (*l);

err_out:;
	return err;
}

terr_t TWI_Rwlock_free (TWI_Rwlock_handle_t l) {
	terr_t err;
	err = TWI_Rwlock_finalize (l);
	TWI_Free (l);
	return err;
}

#ifdef _WIN32
terr_t TWI_Rwlock_init (TWI_Rwlock_handle_t l) {
	InitializeSRWLock (l);

	return TW_SUCCESS;
}
terr_t TWI_Rwlock_finalize (TWI_Rwlock_handle_t l) { return TW_SUCCESS; }
terr_t TWI_Rwlock_rlock (TWI_Rwlock_handle_t l) {
	AcquireSRWLockShared (l);

	return TW_SUCCESS;
}

terr_t TWI_Rwlock_tryrlock (TWI_Rwlock_handle_t l, int *success) {
	BOOLEAN ret;

	ret = TryAcquireSRWLockShared (l);

	if (ret == TRUE)
		*success = 1;
	else
		*success = 0;

	return TW_SUCCESS;
}
terr_t TWI_Rwlock_wlock (TWI_Rwlock_handle_t l) {
	AcquireSRWLockExclusive (l);

	return TW_SUCCESS;
}
terr_t TWI_Rwlock_trywlock (TWI_Rwlock_handle_t l, int *success) {
	BOOLEAN ret;

	ret = TryAcquireSRWLockExclusive (l);

	if (ret == TRUE)
		*success = 1;
	else
		*success = 0;

	return TW_SUCCESS;
}
terr_t TWI_Rwlock_runlock (TWI_Rwlock_handle_t l) {
	ReleaseSRWLockShared (l);

	return TW_SUCCESS;
}
terr_t TWI_Rwlock_wunlock (TWI_Rwlock_handle_t l) {
	ReleaseSRWLockExclusive (l);

	return TW_SUCCESS;
}
#elif defined __USE_UNIX98 || defined __USE_XOPEN2K
terr_t TWI_Rwlock_init (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_init (l, NULL);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Rwlock_finalize (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_destroy (l);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Rwlock_rlock (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_rdlock (l);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Rwlock_tryrdlock (TWI_Rwlock_handle_t l, int *success) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_tryrdlock (l);
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
terr_t TWI_Rwlock_wlock (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_wrlock (l);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Rwlock_trywrlock (TWI_Rwlock_handle_t l, int *success) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_trywrlock (l);
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
terr_t TWI_Rwlock_runlock (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_unlock (l);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
terr_t TWI_Rwlock_wunlock (TWI_Rwlock_handle_t l) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_rwlock_unlock (l);
	if (perr) { RET_OS_ERR (perr) }

err_out:;
	return err;
}
#endif
