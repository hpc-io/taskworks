/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Common internal routines */

#pragma once

#include <config.h>
#include <taskworks.h>

#include "common.h"
#include "driver.h"
#include "twi_err.h"
#include "twi_hash.h"
#include "twi_mem.h"
#include "twi_mutex.h"
#include "twi_nb_list.h"
#include "twi_rwlock.h"
#include "twi_time.h"
#include "twi_ts_vector.h"

#define TWI_TASK_NUM_PRIORITY_LEVEL (TW_TASK_PRIORITY_STANDARD + 1)
#define TW_TASK_PRIORITY_RESERVED	0