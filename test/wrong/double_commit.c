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

int task_fn (void *data);
int task_fn (void *data) {
	int *flag = (int *)data;

	while (*flag) { sleep (1); }

	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int nworker = NUM_WORKERS, ntask = NUM_TASKS;
	int flag;
	TW_Engine_handle_t eng;
	TW_Task_handle_t *task = NULL;

	PRINT_TEST_MSG ("Check if TaskWork can create and free engines");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	task = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);

	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	flag = 1;
	for (i = 0; i < ntask; i++) {
		err = TW_Task_create (task_fn, &flag, TW_TASK_DEP_NULL, i, task + i);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
		// Second commit should fail
		err = TW_Task_commit (task[i], eng);
		EXP_VAL (err, TW_ERR_STATUS, "%d")
	}

	// Turn off the flag so tasks can finish
	flag = 0;
	for (i = 0; i < ntask; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (task);

	PRINT_TEST_RESULT
	return nerr;
}