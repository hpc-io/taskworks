/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread thread driver mutex */

#pragma once

#include <pthread.h>

#include "twposix.h"

#define TWPOSIX_Sem_cb \
	{ TWPOSIX_Sem_create, TWPOSIX_Sem_free, TWPOSIX_Sem_dec, TWPOSIX_Sem_trydec, TWPOSIX_Sem_inc }

terr_t TWPOSIX_Sem_create (TW_Handle_t *sem);
void TWPOSIX_Sem_trydec (TW_Handle_t sem, TWI_Bool_t *success);
void TWPOSIX_Sem_dec (TW_Handle_t sem);
void TWPOSIX_Sem_inc (TW_Handle_t sem);
void TWPOSIX_Sem_free (TW_Handle_t sem);