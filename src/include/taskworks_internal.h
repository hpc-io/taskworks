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

#define TW_N_ERR_CODE 0x400

#ifdef ENABLE_DEBUG
#define TWI_PRINT_ERR(E)
printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, TW_Get_err_msg (E), E);
#else
#define TWI_PRINT_ERR(E)
#endif

#define CHK_RET(R)             \
	{                          \
		if (R != TW_SUCCESS) { \
			PRINT_ERR (R)      \
			goto err_out;      \
		}                      \
	}

#define CHK_ERR CHK_RET (err)

#define RET_ERR(R)    \
	{                 \
		err = R;      \
		PRINT_ERR (R) \
		goto err_out; \
	}

#define CHK_PTR(P)            \
	{                         \
		if (P == NULL) {      \
			err = TW_ERR_MEM; \
			PRINT_ERR (err)   \
			goto err_out;     \
		}                     \
	}

typedef void *TWI_Handle_t;

extern TW_Event_driver_handle_t evt_driver;
extern TW_Driver_handle_t driver;