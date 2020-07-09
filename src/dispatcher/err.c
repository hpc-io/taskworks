/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Error realted functions */

#include "taskworks_internal.h"

const char *TW_Get_err_msg (terr_t err) {
	switch (err) {
		/* No Err */
		case TW_SUCCESS:
			return "Operation finished successfully";
		/* Common */
		case TW_ERR_MEM:
			return "Memory allocation fail";
		/* User related */
		case TW_ERR_INVAL_BACKEND:
			return "Unrecognized engine backend";
		case TW_ERR_INVAL_EVT_BACKEND:
			return "Unrecognized event backend";
		case TW_ERR_INVAL_HANDLE:
			return "Invalid handle";
		case TW_ERR_INCOMPATIBLE_OBJECT:
			return "Objected not compatible with the current backend backend";
		/* Engine related */
		case TW_ERR_THREAD_CREATE:
			return "Cannot create thread";
		case TW_ERR_THREAD_SIG:
			return "Cannot send signal to thread";
		case TW_ERR_NOT_SUPPORTED:
			return "The backend does not support such function";
	}

	return "Unknown error";
}