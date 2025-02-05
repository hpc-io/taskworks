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
#include <mpi.h>
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

char msg[] = "test_msg";
char buf[256];
int event_cb (TW_Event_handle_t __attribute__ ((unused)) evt, TW_Event_args_t *arg, void *data);
int event_cb (TW_Event_handle_t __attribute__ ((unused)) evt, TW_Event_args_t *arg, void *data) {
	terr_t err;
	int mpierr;
	int nerr   = 0;
	int *nerrp = (int *)data;
	int flag, cnt, rcnt;
	MPI_Comm comm;
	MPI_Status stat, rstat;

	err = TW_Event_arg_get_mpi (arg, &comm, &flag, &stat);
	CHECK_ERR
	mpierr = MPI_Get_count (&stat, MPI_CHAR, &cnt);
	if (mpierr != MPI_SUCCESS) { RAISE_ERR ("MPI_Get_count failed") }

	mpierr = MPI_Recv (buf, cnt, MPI_CHAR, stat.MPI_SOURCE, stat.MPI_TAG, comm, &rstat);
	if (mpierr != MPI_SUCCESS) { RAISE_ERR ("MPI_Recv failed") }

	mpierr = MPI_Get_count (&rstat, MPI_CHAR, &rcnt);
	if (mpierr != MPI_SUCCESS) { RAISE_ERR ("MPI_Get_count failed") }

	EXP_VAL (cnt, rcnt, "%d");

	err = TWT_Sem_inc (sem);
	CHECK_ERR

	*nerrp += nerr;

	return 0;
}

int main (int argc, char **argv) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int ret, mpierr;
	atomic_int evtnerr = 0;
	int rank;
	int nworker = NUM_WORKERS;
	TW_Event_args_t arg;
	TW_Event_handle_t evt;
	TW_Engine_handle_t eng;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_SELF, &rank);

	PRINT_TEST_MSG ("Check if file event triggers correctly");

	if (argc > 1) { nworker = atoi (argv[1]); }

	err = TWT_Sem_create (&sem);
	CHECK_ERR

	err = TW_Init (TW_Backend_native, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	err = TW_Event_arg_set_mpi (&arg, MPI_COMM_SELF, MPI_ANY_SOURCE, MPI_ANY_TAG);
	CHECK_ERR
	err = TW_Event_create (event_cb, &evtnerr, arg, &evt);
	CHECK_ERR
	err = TW_Event_commit (evt, eng);
	CHECK_ERR

	mpierr = MPI_Send (msg, (int)strlen (msg), MPI_CHAR, rank, 0, MPI_COMM_SELF);

	err = TWT_Sem_dec (sem);
	CHECK_ERR
	err = TWT_Sem_free (sem);
	CHECK_ERR

	nerr += evtnerr;

	// Avoid comparing '\n' by setting len as strlen(msg)
	ret = strncmp (buf, msg, strlen (msg));
	EXP_VAL (ret, 0, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT

	MPI_Finalize ();
	return nerr;
}
