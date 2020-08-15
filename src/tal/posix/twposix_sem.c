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

#include "twposix_sem.h"

#include <semaphore.h>

terr_t TWPOSIX_Sem_create (TW_Handle_t *sem) {
	terr_t err = TW_SUCCESS;
	int perr;
	sem_t *sp = NULL;

	sp = (sem_t *)TWI_Malloc (sizeof (sem_t));

	perr = sem_init (sp, 0, 0);
	CHECK_PERR

	*sem = sp;

err_out:;
	if (err) { TWI_Free (sp); }
	return err;
}

void TWPOSIX_Sem_trydec (TW_Handle_t sem, TWI_Bool_t *success) {
	int perr;
	perr = sem_trywait (sem);
	if (perr == 0) {
		*success = TWI_TRUE;
	} else {
		*success = TWI_FALSE;
	}
}

void TWPOSIX_Sem_dec (TW_Handle_t sem) { sem_wait (sem); }

void TWPOSIX_Sem_inc (TW_Handle_t sem) { sem_post (sem); }

void TWPOSIX_Sem_free (TW_Handle_t sem) {
	sem_destroy (sem);
	TWI_Free (sem);
}