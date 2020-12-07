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
#include <sys/fcntl.h>
#include "twposix_sem.h"

#include <semaphore.h>
sem_t *make_semaphore(int value);
terr_t TWPOSIX_Sem_create (TW_Handle_t *sem) {
	terr_t err = TW_SUCCESS;
	int perr = TW_SUCCESS;
	sem_t *sp = NULL;
DEBUG
	sp = (sem_t *)TWI_Malloc (sizeof (sem_t));
	sp = make_semaphore(0);
	if(sp == SEM_FAILED)
	    perr = TW_ERR_OS;
	//perr = sem_init (sp, 0, 0);
	CHECK_PERR

	*sem = sp;

err_out:;
	if (err) { TWI_Free (sp); }
	return err;
}

void TWPOSIX_Sem_trydec (TW_Handle_t sem, TWI_Bool_t *success) {
	int perr;
	perr = sem_wait (sem);
	if (perr == 0) {
		*success = TWI_TRUE;
	} else {
		*success = TWI_FALSE;
	}
}

void TWPOSIX_Sem_dec (TW_Handle_t sem) {DEBUG sem_wait (sem); }

void TWPOSIX_Sem_inc (TW_Handle_t sem) {DEBUG sem_post (sem); }

void TWPOSIX_Sem_free (TW_Handle_t sem) {DEBUG
	sem_close(sem);
	//sem_destroy (sem);
	//TWI_Free (sem);
}

sem_t *make_semaphore(int value){DEBUG
    sem_t *semaphore = (sem_t *) malloc(sizeof(sem_t));
//          semaphore = sem_open("/semaphore", O_CREAT, 0644, value);
//          sem_unlink("/semaphore");
          char sem_name[128] = {};
          sprintf(sem_name, "sem-%d", pthread_self());
          semaphore = sem_open(sem_name, O_CREAT, 0644, 0);
          sem_unlink(sem_name);
          return semaphore;
}
