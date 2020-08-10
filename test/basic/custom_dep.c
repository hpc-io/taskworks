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
	int nerr;
	volatile atomic_int *tasks_running;	 // Number of threads in task_fn
	volatile atomic_int *tasks_ran;		 // Number of threads in task_fn
} task_data;

int task_fn (void *data);
int task_fn (void *data) {
	int nerr	  = 0;
	task_data *dp = (task_data *)data;

	++(*(dp->tasks_running));
	++(*(dp->tasks_ran));

	// One at a time
	EXP_VAL (*(dp->tasks_running), 1, "%d")

	--(*(dp->tasks_running));

	dp->nerr = nerr;

	return 0;
}

int Custom_dep_status_change (TW_Task_handle_t task,
							  TW_Task_handle_t parent,
							  int __attribute__ ((unused)) old_status,
							  int new_status,
							  void *dep_data);
int Custom_dep_status_change (TW_Task_handle_t task,
							  TW_Task_handle_t parent,
							  int __attribute__ ((unused)) old_status,
							  int new_status,
							  void *dep_data) {
	TW_Task_handle_t h;
	TW_Task_handle_t *hp = (TW_Task_handle_t *)dep_data;

	h = *hp;

	// Try CAS in my own handle if available
	if (h == TW_HANDLE_NULL || (parent == h && new_status == TW_TASK_STAT_COMPLETED)) {
		if (atomic_compare_exchange_strong (hp, &h, task)) { return TW_TASK_STAT_READY; }
	}

	return TW_TASK_STAT_DEPHOLD;
}

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i, j;
	int status;
	task_data datas[NUM_TASKS];
	atomic_int ran = 0, running = 0;
	TW_Task_handle_t handle = TW_HANDLE_NULL;
	TW_Engine_handle_t eng;
	TW_Task_dep_handler_t dep;
	TW_Task_handle_t task[NUM_TASKS];

	PRINT_TEST_MSG ("Check if Taskworks follows customized dependency");

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_none, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (NUM_WORKERS, &eng);
	CHECK_ERR

	dep.Status_change = Custom_dep_status_change;
	dep.Init		  = NULL;
	dep.Finalize	  = NULL;
	dep.Data		  = &handle;
	dep.Mask		  = TW_TASK_STAT_COMPLETED | TW_TASK_STAT_IDLE;

	for (i = 0; i < NUM_TASKS; i++) {
		datas[i].tasks_running = &running;
		datas[i].tasks_ran	   = &ran;
		err					   = TW_Task_create (task_fn, datas + i, dep, i, task + i);
		CHECK_ERR
	}
	// All to all dep
	for (i = 0; i < NUM_TASKS; i++) {
		for (j = 0; j < NUM_TASKS; j++) {
			if (i != j) {
				err = TW_Task_add_dep (task[i], task[j]);
				CHECK_ERR
			}
		}
	}

	/* Commit all tasks */
	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_commit (task[i], eng);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_wait (task[i], TW_TIMEOUT_NEVER);
		CHECK_ERR
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_get_status (task[i], &status);
		CHECK_ERR

		EXP_VAL (status, TW_TASK_STAT_COMPLETED, "%d")
	}

	for (i = 0; i < NUM_TASKS; i++) {
		err = TW_Task_free (task[i]);
		CHECK_ERR
	}

	EXP_VAL (ran, NUM_TASKS, "%d");

	printf ("main thread engine free\n");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}