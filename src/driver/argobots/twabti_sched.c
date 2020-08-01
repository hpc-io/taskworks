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
int TWABTI_Sched_init (ABT_sched TWI_UNUSED sched, ABT_sched_config config) {
	int abterr, status = ABT_SUCCESS;
	TWABT_Engine_t *ep;

	abterr = ABT_sched_config_read (config, 1, &ep);
	if (status == ABT_SUCCESS) status = abterr;

	abterr = ABT_sched_set_data (sched, (void *)ep);
	if (status == ABT_SUCCESS) status = abterr;

	return ABT_SUCCESS;
}

terr_t TWABTI_Sched_run_single (TWABT_Engine_t *ep, ABT_pool *pools, int *success) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int i;
	TWI_Bool_t locked;
	ABT_unit unit;

	if (ep->evt_driver) {
		TWI_Mutex_trylock (&(ep->evt_lock), &locked);
		if (locked) {
			err = ep->evt_driver->Loop_check_events (ep->evt_loop, 100000);
			CHECK_ERR
			TWI_Mutex_unlock (&(ep->evt_lock));
		}
	}

	// Pull one task
	*success = 0;
	for (i = 0; i < TWI_TASK_NUM_PRIORITY_LEVEL; i++) {
		abterr = ABT_pool_pop (pools[i], &unit);
		CHECK_ABTERR
		if (unit != ABT_UNIT_NULL) {
			abterr = ABT_xstream_run_unit (unit, pools[i]);
			CHECK_ABTERR
			*success = 1;
			break;
		}
	}

err_out:;
	return err;
}

void TWABTI_Sched_run (ABT_sched sched) {
	terr_t err = TW_SUCCESS;
	int abterr;
	int havejob;
	// uint32_t work_count = 0;
	ABT_pool pools[TWI_TASK_NUM_PRIORITY_LEVEL];
	ABT_bool stop;
	TWABT_Engine_t *ep;

	abterr = ABT_sched_get_data (sched, (void **)(&ep));
	CHECK_ABTERR

	abterr = ABT_sched_get_pools (sched, TWI_TASK_NUM_PRIORITY_LEVEL, 0, pools);
	CHECK_ABTERR

	while (1) {
		err = TWABTI_Sched_run_single (ep, pools, &havejob);
		CHECK_ERR

		abterr = ABT_xstream_check_events (sched);
		CHECK_ABTERR

		abterr = ABT_sched_has_to_stop (sched, &stop);
		CHECK_ABTERR
		if (stop == ABT_TRUE) { break; }
	}

	/* //TODO: Check for event
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
 * */
int TWABTI_Sched_finalize (ABT_sched TWI_UNUSED sched) { return ABT_SUCCESS; }