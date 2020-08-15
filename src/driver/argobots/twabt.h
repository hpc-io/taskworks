/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver functions */

#pragma once

#include <time.h>
// Time.h must go first to eliminate abt.h warning

#include <abt.h>

#include "driver.h"
#include "taskworks_internal.h"
#include "twabt_engine.h"
#include "twabt_event.h"
#include "twabt_task.h"

#ifdef TWI_DEBUG
#define TWABTI_PRINT_ERR(E)                                                                      \
	{                                                                                            \
		char errmsg[256];                                                                        \
		size_t len;                                                                              \
		int ret;                                                                                 \
                                                                                                 \
		ret = ABT_error_get_str (E, NULL, &len);                                                 \
		if (ret == ABT_SUCCESS && len < 256) {                                                   \
			ret = ABT_error_get_str (E, errmsg, NULL);                                           \
			if (ret == ABT_SUCCESS) {                                                            \
				printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, errmsg, E);     \
			} else {                                                                             \
				printf ("Error at line %d in %s: Unknown argobots error (%d)\n", __LINE__,       \
						__FILE__, E);                                                            \
			}                                                                                    \
		} else {                                                                                 \
			printf ("Error at line %d in %s: Unknown argobots error (%d)\n", __LINE__, __FILE__, \
					E);                                                                          \
		}                                                                                        \
	}
#else
#define TWABTI_PRINT_ERR(E)
#endif

#define CHK_ABTRET(R)                      \
	{                                      \
		if (R != ABT_SUCCESS) {            \
			TWABTI_PRINT_ERR (R)           \
			err = TWABT_Err_to_tw_err (R); \
			PRINT_ERR (err)                \
			goto err_out;                  \
		}                                  \
	}

#define CHECK_ABTERR CHK_ABTRET (abterr)

extern TWI_Disposer_handle_t TWABTI_Disposer;

extern TWI_Ts_vector_handle_t TWABTI_Tasks;
extern TWI_Ts_vector_handle_t TWABTI_Engines;
extern TWI_Ts_vector_handle_t TWABTI_Events;

/* Init callbacks */
terr_t TWABT_Init (int *argc, char ***argv);
terr_t TWABT_Finalize (void);

/* Error handling */
terr_t TWABT_Err_to_tw_err (int abterr);

/* Schedulers */
int TWABTI_Sched_init (ABT_sched sched, ABT_sched_config config);
void TWABTI_Sched_run (ABT_sched sched);
int TWABTI_Sched_finalize (ABT_sched sched);
terr_t TWABTI_Sched_run_single (TWABT_Engine_t *ep, ABT_pool *pools, int *success);