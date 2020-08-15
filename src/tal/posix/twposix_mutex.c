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

terr_t TWPOSIX_Mutex_create (TW_handle_t *m) {
	terr_t err = TW_SUCCESS;
	int perr;
	pthread_mutex_t *mp = NULL;

	mp = (pthread_mutex_t *)TWI_Malloc (sizeof (pthread_mutex_t));
	CHECK_PTR (mp);

	perr = pthread_mutex_init (mp, NULL);
	CHECK_PERR

	*m = (TW_handle_t)mp;

err_out:;
	if (err) { TWI_Free (mp); }
	return err;
}

void TWPOSIX_Mutex_free (TW_handle_t m) {
	pthread_mutex_destroy ((pthread_mutex_t *)m);
	TWI_Free (m);
}
void TWPOSIX_Mutex_lock (TW_handle_t m) { pthread_mutex_lock ((pthread_mutex_t *)m); }
void TWPOSIX_Mutex_trylock (TW_handle_t m, TWI_Bool_t *success) {
	int perr;

	perr = pthread_mutex_trylock ((pthread_mutex_t *)m);
	if (perr == 0)
		*success = 1;
	else {
		*success = 0;
	}
}
void TWPOSIX_Mutex_unlock (TW_handle_t m) { pthread_mutex_unlock ((pthread_mutex_t *)m); }