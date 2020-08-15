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

#define NUM_TASKS 10

int task_fn (void *data);
int task_fn (void *data) {
	++(*((atomic_int *)data));
	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int ntask = NUM_TASKS;
	atomic_int ctr;
	TW_Engine_handle_t eng;
	TW_Task_handle_t *task = NULL;

	PRINT_TEST_MSG ("Check if the engine can run tasks with the main thread when needed");

	if (argc > 2) { ntask = atoi (argv[2]); }
	task = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (0, &eng);
	CHECK_ERR

	ctr = 0;
	for (i = 0; i < ntask; i++) {
		err = TW_Task_create (task_fn, &ctr, TW_TASK_DEP_ALL_COMPLETE, 0, task + i);
		CHECK_ERR

		if (i) {
			err = TW_Task_add_dep (task[i], task[i - 1]);
			CHECK_ERR
		}
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	// Should run 0 ~ ntask/2
	err = TW_Task_wait (task[ntask / 2], TW_TIMEOUT_NEVER);
	CHECK_ERR
	for (i = 0; i <= ntask / 2; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}
	EXP_VAL (ctr, (ntask / 2 + 1), "%d");

	// Run 1 additional task
	err = TW_Engine_progress (eng);
	CHECK_ERR
	if (ntask > 2) {
		EXP_VAL (ctr, (ntask / 2 + 2), "%d");
	} else {
		EXP_VAL (ctr, (ntask / 2 + 1), "%d");  // No more task to run
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

	EXP_VAL (ctr, ntask, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (task);

	PRINT_TEST_RESULT
	return nerr;
}