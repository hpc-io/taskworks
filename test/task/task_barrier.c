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

int ntask = NUM_TASKS;

typedef struct task_data {
	int nerr;
	atomic_int *ctr;
} task_data;

int task_fn (void *data);
int task_fn (void *data) {
	int nerr = 0;
	int v;
	task_data *dp = (task_data *)data;

	v = *((atomic_int *)dp->ctr);
	EXP_COND (v < ntask)

	++(*((atomic_int *)(dp->ctr)));

	dp->nerr = nerr;

	return 0;
}

int task_after_fn (void *data);
int task_after_fn (void *data) {
	int nerr = 0;
	int v;
	task_data *dp = (task_data *)data;

	v = *((atomic_int *)dp->ctr);
	EXP_VAL (v, ntask, "%d")

	*((atomic_int *)(dp->ctr)) += ntask;

	dp->nerr = nerr;

	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	int nworker = NUM_WORKERS;
	atomic_int ctr;
	TW_Engine_handle_t eng;
	task_data *datas	   = NULL;
	TW_Task_handle_t *task = NULL;
	TW_Task_handle_t bar;

	PRINT_TEST_MSG (
		"Check if all task before a barrier task finishes before tasks after the barrier task "
		"starts");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	task  = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * ((size_t)ntask + 1));
	datas = (task_data *)malloc (sizeof (task_data) * ((size_t)ntask + 1));

	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	ctr = 0;
	for (i = 0; i < ntask; i++) {
		datas[i].ctr  = &ctr;
		datas[i].nerr = 0;
		err			  = TW_Task_create (task_fn, datas + i, TW_TASK_DEP_NULL, i, task + i);
		CHECK_ERR
	}

	// Create barrier task
	err = TW_Task_create_barrier (NULL, TW_TASK_TAG_ANY, ntask + 1, &bar);
	CHECK_ERR

	// Task after barrier
	datas[ntask].ctr  = &ctr;
	datas[ntask].nerr = 0;
	err = TW_Task_create (task_after_fn, datas + ntask, TW_TASK_DEP_ALL_COMPLETE, ntask + 2,
						  task + ntask);
	CHECK_ERR
	err = TW_Task_add_dep (task[ntask], bar);
	CHECK_ERR

	// Commit barrier task
	err = TW_Task_commit_barrier (bar);
	CHECK_ERR

	// Commit in reverse order
	for (i = ntask; i > -1; i--) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	for (i = 0; i <= ntask; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
		nerr += datas[i].nerr;
	}

	for (i = 0; i <= ntask; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR
		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}

	for (i = 0; i <= ntask; i++) {
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}

	EXP_VAL (ctr, ntask * 2, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (datas);
	free (task);

	PRINT_TEST_RESULT
	return nerr;
}