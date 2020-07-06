/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* User level Task API */

#include "taskworks_internal.h"

typedef struct TW_task {
	OPA_int_t status;  // Status of the task

	TW_Task_handler_t cb;  // Function to run
	void *cb_data;		   // Input parameters to the task function
	terr_t ret;			   // Return value of the task function, only meanningfull when
				 // stat is TW_Task_stat_completed # use void

	TW_Task_dep_handler_t dep_cb;  // Function that decide the status of the task based on
								   // dependency
	void *dep_data;				   // Input parameters to the dependency function function

	// TODO: There must be defualt hard-coded dep handler that takes constant time

	int tag;  // Optional tag to be used by the application, do we need it?
} TW_task;