/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Error realted functions */

#include "taskworks_internal.h"

const char *TW_Get_err_msg (terr_t err) {
	switch (err) {
		/* No Err */
		case TW_ERR_SUCCESS:
			return "Success";
		/* Memory related */
		case TW_Engine_OOM:
			return "Memory allocation fail";
		/* Thread related */
		case TW_Engine_THREAD_CREATE:
			return "Cannot create thread";
		case TW_Engine_THREAD_SIG:
			return "Cannot send signal to thread";
	}
	return "Unknown error";
}