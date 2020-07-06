/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Error codes */

#pragma once

#include "taskworks.h"

/* No Err */
#define TW_ERR_SUCCESS 0x0

/* Memory related */
#define TW_Engine_OOM 0x10 /* Out of memory */

/* Thread related */
#define TW_Engine_THREAD_CREATE 0x20 /* Cannot create thread */
#define TW_Engine_THREAD_SIG	0x21 /* Cannot signal thread */

const char *TW_Get_err_msg (terr_t err);