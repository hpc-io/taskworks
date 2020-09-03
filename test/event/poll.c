/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Test engine APIs */

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <signal.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <twtest.h>

#define NUM_WORKERS 4

TWT_Semaphore sem;

TW_Event_poll_response_t poll_check (void *data);
TW_Event_poll_response_t poll_check (void *data) {
	int *flag = (int *)data;

	if (*flag) {
		*flag = 0;
		return TW_Event_poll_response_trigger;
	}
	return TW_Event_poll_response_pending;
}

int event_cb (TW_Event_handle_t __attribute__ ((unused)) evt,
			  TW_Event_args_t __attribute__ ((unused)) * arg,
			  void *data);
int event_cb (TW_Event_handle_t __attribute__ ((unused)) evt,
			  TW_Event_args_t __attribute__ ((unused)) * arg,
			  void *data) {
	terr_t err;
	int nerr   = 0;
	int *nerrp = (int *)data;

	err = TWT_Sem_inc (sem);
	CHECK_ERR

	*nerrp += nerr;

	return 0;
}

int main (int argc, char **argv) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int i;
	atomic_int evtnerr = 0;
	int flag		   = 0;
	int nworker		   = NUM_WORKERS;
	TW_Event_args_t arg;
	TW_Event_handle_t evt;
	TW_Engine_handle_t eng;

	PRINT_TEST_MSG ("Check if customized polling events triggers correctly");

	if (argc > 1) { nworker = atoi (argv[1]); }

	err = TWT_Sem_create (&sem);
	CHECK_ERR

	err = TW_Init (TW_Backend_native, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	err = TW_Event_arg_set_poll (&arg, poll_check, &flag);
	CHECK_ERR
	err = TW_Event_create (event_cb, &evtnerr, arg, &evt);
	CHECK_ERR
	err = TW_Event_commit (evt, eng);
	CHECK_ERR

	for (i = 0; i < 2; i++) {
		flag = 1;
		err	 = TWT_Sem_dec (sem);
		CHECK_ERR
	}

	err = TWT_Sem_free (sem);
	CHECK_ERR

	nerr += evtnerr;

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT

	return nerr;
}
