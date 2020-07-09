/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Error codes */

#pragma once

#include "taskworks.h"

/* No Err */
#define TW_SUCCESS 0x0

/* Common */
#define TW_ERR_MEM 0x01 /* Out of memory */

/* User API related */
#define TW_ERR_INVAL_BACKEND	   0x10 /* Invalid backend selection */
#define TW_ERR_INVAL_EVT_BACKEND   0x11 /* Invalid event backend selection */
#define TW_ERR_INVAL_HANDLE		   0x12 /* Invalid handle */
#define TW_ERR_INCOMPATIBLE_OBJECT 0x13 /* Objected created by a different backend */

/* Engine driver related */
#define TW_ERR_THREAD_CREATE 0x100 /* Cannot create thread */
#define TW_ERR_THREAD_SIG	 0x101 /* Cannot signal thread */
#define TW_ERR_NOT_SUPPORTED 0x102 /* The backend does not support such function*/

const char *TW_Get_err_msg (terr_t err);