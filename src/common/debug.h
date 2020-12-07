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
#ifdef TWI_DEBUG
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

#define DEBUG_ENTER_FUNC(L) DEBUG_PRINTF (L, "Entering %s\n", __func__)
#define DEBUG_EXIT_FUNC(L)	DEBUG_PRINTF (L, "Leaving %s\n", __func__)
#define DEBUG printf("Tid = %d, %s:%s:%d\n", pthread_self(), __FILE__, __func__, __LINE__); usleep(50*1000);
extern int TWI_Debug_level;

int TWI_Get_tid (void);
