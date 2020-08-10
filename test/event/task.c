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

#define NUM_WORKERS 1
#define NUM_TASKS	1

TWT_Semaphore sem;

int task_fn (void *data);
int task_fn (void *data) {
	++(*((atomic_int *)data));
	return 0;
}

void event_fn (TW_Task_handle_t task, int status);
void event_fn (TW_Task_handle_t task, int status) {
	void *data;

	TW_Task_get_data (task, &data);
	++(*((atomic_int *)data));

	TWT_Sem_inc (sem);

	return 0;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	int status;
	atomic_int ctr;
	TW_Engine_handle_t eng;
	TW_Event_args_t arg;
	TW_Task_handle_t task[NUM_TASKS];

	PRINT_TEST_MSG ("Check if TaskWork can create and free engines");

	err = TWT_Sem_create (&sem);
	CHECK_ERR

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (NUM_WORKERS, &eng);
	CHECK_ERR

	ctr = 0;
	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_create_ex (task_fn, &ctr, TW_TASK_DEP_NULL, 0, TW_TASK_PRIORITY_STANDARD,
								 event_fn, TW_TASK_STAT_COMPLETED, task + i);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		// err = TWT_Sem_dec (sem);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}

	err = TWT_Sem_free (sem);
	CHECK_ERR

	EXP_VAL (ctr, NUM_TASKS * 2, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}