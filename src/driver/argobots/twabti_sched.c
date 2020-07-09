/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver scheduler implementation */

#include "twabt.h"

/**
 * @brief  Initialize the scheduler
 * @note
 * @param  sched: Scheduler to initialize
 * @param  config:
 * @retval
 */
int TWABTI_Sched_init (ABT_sched sched, ABT_sched_config config) { return ABT_SUCCESS; }

void TWABTI_Sched_run (ABT_sched sched) {
	terr_t err;
	int abterr;
	// uint32_t work_count = 0;
	ABT_pool pool;
	ABT_unit unit;
	int target;
	ABT_bool stop;

	abterr = ABT_sched_get_pools (sched, 1, 0, &pool);
	CHECK_ABTERR

	while (1) {
		abterr = ABT_pool_pop (pool, &unit);
		CHECK_ABTERR
		if (unit != ABT_UNIT_NULL) { ABT_xstream_run_unit (unit, pool); }

		abterr = ABT_sched_has_to_stop (sched, &stop);
		CHECK_ABTERR
		if (stop == ABT_TRUE) { break; }

		abterr = ABT_xstream_check_events (sched);
		CHECK_ABTERR
	}

	/* //TODO: CHeck for event
	work_count = 0;
	ABT_sched_has_to_stop (sched, &stop);
	if (stop == ABT_TRUE) break;
	ABT_xstream_check_events (sched);
	*/
err_out:;
}

/**
 * @brief  Free the scheduler
 * @note
 * @param  sched: Scheduler to free
 * @retval
 */
int TWABTI_Sched_finalize (ABT_sched sched) { return ABT_SUCCESS; }