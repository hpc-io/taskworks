/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Test task APIs */

#include <stdatomic.h>
#include <twtest.h>

#define NUM_WORKERS 4
#define NUM_TASKS	10

typedef struct task_data {
	atomic_int ctr;
	atomic_int nerr;
} task_data;

int task_high (void *data);
int task_high (void *data) {
	int nerr	  = 0;
	task_data *dp = (task_data *)data;

	EXP_COND ((dp->ctr < 0))
	++(dp->ctr);

	dp->nerr += nerr;

	return 0;
}

int task_low (void *data);
int task_low (void *data) {
	int nerr	  = 0;
	task_data *dp = (task_data *)data;

	EXP_COND ((dp->ctr >= 0))
	++(dp->ctr);

	dp->nerr += nerr;

	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int nworker = NUM_WORKERS, ntask = NUM_TASKS;
	task_data data;
	TW_Engine_handle_t eng;
	TW_Task_handle_t *task = NULL;

	PRINT_TEST_MSG ("Check if TaskWork follows task priority");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	task = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);

	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	/* There is no way to guarantee execution order
	 * Don't create any worker
	 */
	err = TW_Engine_create (0, &eng);
	CHECK_ERR

	data.nerr = 0;
	data.ctr  = -(ntask / 2);
	for (i = 0; i < ntask; i++) {
		if (i & 1) {
			err = TW_Task_create_ex (task_low, &data, TW_TASK_DEP_NULL, i,
									 TW_TASK_PRIORITY_STANDARD, NULL, 0, task + i);
			CHECK_ERR
		} else {
			err = TW_Task_create_ex (task_high, &data, TW_TASK_DEP_NULL, i, TW_TASK_PRIORITY_URGENT,
									 NULL, 0, task + i);
			CHECK_ERR
		}
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Engine_progress (eng);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}

	EXP_VAL (data.ctr, ntask / 2, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (task);

	PRINT_TEST_RESULT
	return nerr;
}