/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread driver implementation */

#include "twposix.h"

#if defined __USE_UNIX98 || defined __USE_XOPEN2K

terr_t TWPOSIX_Rwlock_create (TW_handle_t *r) {
	terr_t err = TW_SUCCESS;
	int perr;
	pthread_rwlock_t *rp = NULL;

	rp = (pthread_rwlock_t *)TWI_Malloc (sizeof (pthread_rwlock_t));
	CHECK_PTR (rp);

	perr = pthread_rwlock_init (rp, NULL);
	CHECK_PERR

	*r = (TW_handle_t)rp;

err_out:;
	if (err) { TWI_Free (rp); }
	return err;
}

void TWPOSIX_Rwlock_free (TW_handle_t r) { TWI_Free (m); }
void TWPOSIX_Rwlock_rlock (TW_handle_t r) { pthread_rwlock_rdlock ((pthread_rwlock_t *)r); }
void TWPOSIX_Rwlock_tryrlock (TW_handle_t r, TWI_Bool_t *success) {
	int perr;

	perr = pthread_rwlock_tryrdlock ((pthread_rwlock_t *)r);
	if (perr == 0)
		*success = 1;
	else {
		CHECK_MERR
		*success = 0;
	}
}
void TWPOSIX_Rwlock_runlock (TW_handle_t r) { pthread_rwlock_unlock ((pthread_rwlock_t *)r); }
void TWPOSIX_Rwlock_wlock (TW_handle_t r) { pthread_rwlock_wrlock ((pthread_rwlock_t *)r); }
void TWPOSIX_Rwlock_trywlock (TW_handle_t r, TWI_Bool_t *success) {
	int perr;

	perr = pthread_rwlock_trywrlock ((pthread_rwlock_t *)r);
	if (perr == 0)
		*success = 1;
	else {
		CHECK_MERR
		*success = 0;
	}
}
void TWPOSIX_Rwlock_wunlock (TW_handle_t r) { pthread_rwlock_unlock ((pthread_rwlock_t *)r); }

#endif
