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

TWT_Semaphore sem;

int task_fn (void *data);
int task_fn (void *data) {
	++(*((atomic_int *)data));
	return 0;
}

void event_fn (TW_Task_handle_t task, int __attribute__ ((unused)) status);
void event_fn (TW_Task_handle_t task, int __attribute__ ((unused)) status) {
	void *data;

	TW_Task_get_data (task, &data);
	++(*((atomic_int *)data));

	TWT_Sem_inc (sem);
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int nworker = NUM_WORKERS, ntask = NUM_TASKS;
	atomic_int ctr;
	TW_Engine_handle_t eng;
	TW_Task_handle_t *task;

	PRINT_TEST_MSG ("Check if TaskWork can create and free engines");

	if (argc > 1) { nworker = atoi (argv[1]); }
	if (argc > 2) { ntask = atoi (argv[2]); }
	task = (TW_Task_handle_t *)malloc (sizeof (TW_Task_handle_t) * (size_t)ntask);

	err = TWT_Sem_create (&sem);
	CHECK_ERR
	DEBUG
	err = TW_Init (TW_Backend_native, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR
    DEBUG
	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR
    DEBUG
	ctr = 0;
	for (i = 0; i < ntask; i++) {
		err = TW_Task_create_ex (task_fn, &ctr, TW_TASK_DEP_NULL, i, TW_TASK_PRIORITY_STANDARD,
								 event_fn, TW_TASK_STAT_COMPLETED, task + i);
	    DEBUG
		CHECK_ERR
	}
    DEBUG
	for (i = 0; i < ntask; i++) {
	    DEBUG
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}
    DEBUG
	for (i = 0; i < ntask; i++) {
	    DEBUG
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}
    DEBUG
	for (i = 0; i < ntask; i++) {
	    DEBUG
		err = TWT_Sem_dec (sem);
		CHECK_ERR
	}
    DEBUG
	for (i = 0; i < ntask; i++) {
	    DEBUG
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}
    DEBUG
	err = TWT_Sem_free (sem);
	CHECK_ERR
    DEBUG
	EXP_VAL (ctr, ntask * 2, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	free (task);

	PRINT_TEST_RESULT
	return nerr;
}
