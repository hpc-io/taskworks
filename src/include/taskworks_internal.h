/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Function and macros for internal use */

#pragma once

#include "taskworks.h"
#include "twi_common.h"
#include "twi_driver.h"

#define TW_Engine_N_ERR_CODE 0x400

#ifdef ENABLE_DEBUG
#define PRINT_ERR(E)
printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, TW_Get_err_msg (E), E);
#else
#define PRINT_ERR(E)
#endif

#define TW_Engine_CHECK_RET(R)     \
	{                              \
		if (R != TW_ERR_SUCCESS) { \
			PRINT_ERR (R)          \
			goto err_out;          \
		}                          \
	}

#define TW_Engine_CHECK_ERR TW_Engine_CHECK_RET (err)

#define TW_Engine_CHECK_PTR(P)        \
	{                                 \
		if (P == NULL) {              \
			PRINT_ERR (TW_Engine_OOM) \
			goto err_out;             \
		}                             \
	}

typedef void *TWI_Handle_t;