/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Non-thread-safe hash */

#pragma once

#include <stdlib.h>
#include <taskworks_internal.h>
#ifdef ENABLE_DEBUG
#define DEBUG_PRINTF(L, M, ...)                         \
	{                                                   \
		if (TWI_Debug_level >= L) {                     \
			int tid = TWI_Get_tid ();                   \
			printf ("Thread %d: " M, tid, __VA_ARGS__); \
		}                                               \
	}
#else
#define DEBUG_PRINTF(L, M, ...) \
	{}
#endif

#define DEBUG_ENTER_FUNC(L) DEBUG_PRINTF (L, "Entering %s", __func__)
#define DEBUG_EXIT_FUNC(L)	DEBUG_PRINTF (L, "Leaving %s", __func__)

extern int TWI_Debug_level;

int TWI_Get_tid (void);
