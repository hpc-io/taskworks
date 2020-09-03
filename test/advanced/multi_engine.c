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
	TW_Task_handle_t tp;
	atomic_int nerr;
} task_data;

int task_fn (void *data);
int task_fn (void *data) {
	++(*((atomic_int *)data));
	return 0;
}

int task_fn_wait (void *data);
int task_fn_wait (void *data) {
	terr_t err;
	int nerr	  = 0;
	task_data *dp = (task_data *)data;

	err = TW_Task_wait (dp->tp, TW_TIMEOUT_NEVER);
	CHECK_ERR

	dp->nerr += nerr;
	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int nworker = NUM_WORKERS, ntask = NUM_TASKS;
	atomic_int ctr;
	TW_Engine_handle_t eng[2];
	task_data *data		   = NULL;
	TW_Task_handle_t *task = NULL;

	PRINT_TEST_MSG ("Check if TaskWork can create and free engines");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	data = (task_data *)malloc (sizeof (task_data) * (size_t)ntask);
	task = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);

	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	// First engine has no workers
	err = TW_Engine_create (0, eng);
	CHECK_ERR
	err = TW_Engine_create (nworker, eng + 1);
	CHECK_ERR

	ctr = 0;
	for (i = 0; i < ntask; i++) {
		data[i].nerr = 0;
		err			 = TW_Task_create (task_fn, &ctr, TW_TASK_DEP_NULL, i, &(data[i].tp));
		CHECK_ERR
		err = TW_Task_create (task_fn_wait, data + i, TW_TASK_DEP_NULL, i, task + i);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_commit (data[i].tp, eng[0]);
		CHECK_ERR
		// Help run the task commited to the first engine by waiting on it
		err = TW_Task_commit (task[i], eng[1]);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_get_status (data[i].tp, &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}

	for (i = 0; i < ntask; i++) {
		err = TW_Task_free (data[i].tp);
		CHECK_ERR
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}

	EXP_VAL (ctr, ntask, "%d");

	err = TW_Engine_free (eng[0]);
	CHECK_ERR
	err = TW_Engine_free (eng[1]);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (data);
	free (task);

	PRINT_TEST_RESULT
	return nerr;
}