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

terr_t TWPOSIX_Tls_create (TW_handle_t *k) {
	terr_t err = TW_SUCCESS;
	int perr;
	pthread_key_t *kp = NULL;

	kp = (pthread_key_t *)TWI_Malloc (sizeof (pthread_key_t));
	CHECK_PTR (kp)

	perr = pthread_key_create (kp, NULL);
	CHECK_PERR

	*k = kp;

err_out:;
	if (err) { TWI_Free (kp); }
	return err;
}

void TWPOSIX_Tls_free (TW_handle_t k) {
	pthread_key_delete (*(pthread_key_t *)k);
	TWI_Free (k);
}

void *TWPOSIX_Tls_load (TW_handle_t k) { return pthread_getspecific (*(pthread_key_t *)k); }
void TWPOSIX_Tls_store (TW_handle_t k, void *data) {
	pthread_setspecific (*(pthread_key_t *)k, data);
}
