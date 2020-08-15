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

TW_Thread_driver_t TWPOSIX_Driver = {
	NULL,			NULL,		   TWPOSIX_Create,	 TWPOSIX_Join,
	TWPOSIX_Cancel, TWPOSIX_Exit,  TWPOSIX_Mutex_cb, TWPOSIX_Rwlock_cb,
	TWPOSIX_Sem_cb, TWPOSIX_Tls_cb};

/*
typedef struct TWPOSIX_Thread_t {
	pthread_t thread;
	TWTD_driver_cb_t main;
	void *data;
} TWPOSIX_Thread_t;
*/
/*
void *TWPOSIX_Thread_fn (void *data) {
	TWPOSIX_Thread_t *tp = (TWPOSIX_Thread_t *)data;

	tp->main (tp->data);

	return NULL;
}
*/

terr_t TWPOSIX_Init (int TWI_UNUSED *argc, char TWI_UNUSED **argv[]) { return TW_SUCCESS; }
terr_t TWPOSIX_Finalize (void) { return TW_SUCCESS; }
terr_t TWPOSIX_Create (TWTD_driver_cb_t main, void *data, TW_handle_t *ht) {
	terr_t err = TW_SUCCESS;
	int perr;
	pthread_t *tp;

	tp = (pthread_t *)TWI_Malloc (sizeof (pthread_t));
	CHECK_PTR (tp);

	perr = pthread_create (tp, NULL, main, data);
	CHECK_PERR

	*ht = (TW_handle_t)tp;

err_out:;
	return err;
}
terr_t TWPOSIX_Join (TW_handle_t ht, void **ret) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_join (*((pthread_t *)ht), ret);
	CHECK_PERR

	TWI_Free (ht);

err_out:;
	return err;
}
terr_t TWPOSIX_Cancel (TW_handle_t ht) {
	terr_t err = TW_SUCCESS;
	int perr;

	perr = pthread_cancel (*((pthread_t *)ht));
	CHECK_PERR

	TWI_Free (ht);

err_out:;
	return err;
}
void TWPOSIX_Exit (void *ret) { pthread_exit (ret); }