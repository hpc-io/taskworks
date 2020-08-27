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
#define TW_ERR_MEM 0x10 /* Out of memory */
#define TW_ERR_OS  0x11 /* OS error */

/* User API related */
#define TW_ERR_INVAL			   0x100 /* Invalid argument */
#define TW_ERR_INVAL_BACKEND	   0x101 /* Invalid backend selection */
#define TW_ERR_INVAL_EVT_BACKEND   0x102 /* Invalid event backend selection */
#define TW_ERR_INVAL_HANDLE		   0x103 /* Invalid handle */
#define TW_ERR_INCOMPATIBLE_OBJECT 0x104 /* Objected created by a different backend */
#define TW_ERR_NOT_FOUND		   0x105 /* Object not found */
#define TW_ERR_STATUS			   0x106 /* Wrong status */
#define TW_ERR_TIMEOUT			   0x107 /* Timeout */
#define TW_ERR_DEP_INIT			   0x108 /* Dependency state initializer returns non-zero */
#define TW_ERR_POLL				   0x109 /* Event poll function returns negative */

/* Engine driver related */
#define TW_ERR_THREAD_CREATE 0x200 /* Cannot create thread */
#define TW_ERR_THREAD_SIG	 0x201 /* Cannot signal thread */
#define TW_ERR_NOT_SUPPORTED 0x202 /* The backend does not support such function*/

const char *TW_Get_err_msg (terr_t err);