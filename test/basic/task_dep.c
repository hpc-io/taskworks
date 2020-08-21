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
	int tid;
	int nerr;
	int *pre;
} task_data;

int task_fn (void *data);
int task_fn (void *data) {
	int nerr	  = 0;
	task_data *dp = (task_data *)data;

	if (dp->tid) { EXP_VAL (*(dp->pre), dp->tid - 1, "%d") }

	*(dp->pre) = dp->tid;
	dp->nerr   = nerr;

	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int nworker = NUM_WORKERS, ntask = NUM_TASKS;
	int last = NUM_TASKS;
	TW_Engine_handle_t eng;
	task_data *datas	   = NULL;
	TW_Task_handle_t *task = NULL;

	PRINT_TEST_MSG ("Check if TaskWork follows task dependency");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	task  = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);
	datas = (task_data *)malloc (sizeof (task_data) * (size_t)ntask);

	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	for (i = 0; i < ntask; i++) {
		datas[i].tid = i;
		datas[i].pre = &last;
		err			 = TW_Task_create (task_fn, datas + i, TW_TASK_DEP_ALL_COMPLETE, i, task + i);
		CHECK_ERR

		if (i) {
			err = TW_Task_add_dep (task[i], task[i - 1]);
			CHECK_ERR
		}
	}

	/* Commit in reverse order, don't commit task 0 */
	for (i = ntask - 1; i > 0; i--) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	/* No task should run */
	for (i = 1; i < ntask; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR

		EXP_VAL (status, TW_TASK_STAT_DEPHOLD, "%d")
	}

	/* Now commit the first task */
	err = TW_Task_commit (task[0], eng);
	CHECK_ERR

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

	for (i = 0; i < ntask; i++) { nerr += datas[i].nerr; }

	EXP_VAL (last, ntask - 1, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (datas);
	free (task);

	PRINT_TEST_RESULT
	return nerr;
}