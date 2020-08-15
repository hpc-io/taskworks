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

#include <twtest.h>

#define NUM_WORKERS 4

int main (int argc, char *argv[]) {
	terr_t err	= TW_SUCCESS;
	int nerr	= 0;
	int nworker = NUM_WORKERS;
	TW_Engine_handle_t eng;

	PRINT_TEST_MSG (
		"Check if TaskWork can create and free engines with event loop integration backend");

	if (argc > 1) { nworker = atoi (argv[1]); }

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}