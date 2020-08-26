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

int main (int argc, char *argv[]) {
	terr_t err = TW_SUCCESS;
	TW_Engine_handle_t eng;
	int nerr = 0;

	PRINT_TEST_MSG ("Check if TaskWork can be initialized with event backend");

	err = TW_Init (TW_Backend_native, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR
	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}