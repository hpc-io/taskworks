/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread thread driver */

#pragma once

#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "tal.h"
#include "taskworks_internal.h"
#include "twposix_mutex.h"
#include "twposix_rwlock.h"
#include "twposix_sem.h"
#include "twposix_tls.h"

#ifdef TWI_DEBUG
#define TWPOSIXI_PRINT_ERR(E)                                                                    \
	{                                                                                            \
		char *errmsg = NULL;                                                                     \
                                                                                                 \
		errmsg = strerror (E);                                                                   \
		if (errmsg) {                                                                            \
			printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, errmsg, E);         \
		} else {                                                                                 \
			printf ("Error at line %d in %s: Unknown argobots error (%d)\n", __LINE__, __FILE__, \
					E);                                                                          \
		}                                                                                        \
	}
#else
#define TWPOSIXI_PRINT_ERR(E)
#endif

#define CHK_POSIXRET(R)                      \
	{                                        \
		if (R != 0) {                        \
			TWPOSIXI_PRINT_ERR (R)           \
			err = TWPOSIX_Err_to_tw_err (R); \
			PRINT_ERR (err)                  \
			goto err_out;                    \
		}                                    \
	}

#define CHECK_PERR CHK_POSIXRET (perr)

typedef pthread_t TWPOSIX_Thread_t;
typedef TWPOSIX_Thread_t *TWPOSIX_Thread_handle_t;

terr_t TWPOSIX_Init (int *argc, char **argv[]);
terr_t TWPOSIX_Finalize (void);
terr_t TWPOSIX_Create (TWTD_driver_cb_t main, void *data, TW_handle_t *ht);
terr_t TWPOSIX_Join (TW_handle_t ht, void **ret);
terr_t TWPOSIX_Cancel (TW_handle_t ht);
void TWPOSIX_Exit (void *ret);

terr_t TWPOSIX_Err_to_tw_err (int TWI_UNUSED perr);
